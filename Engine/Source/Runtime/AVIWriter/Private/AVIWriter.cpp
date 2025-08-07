// Copyright Epic Games, Inc. All Rights Reserved.

/*=============================================================================
	AVIWriter.cpp: AVI creation implementation.
=============================================================================*/
#include "AVIWriter.h"
#include "HAL/PlatformFilemanager.h"
#include "HAL/FileManager.h"
#include "Misc/ScopeLock.h"
#include "Async/Async.h"
#include "Modules/ModuleManager.h"

DEFINE_LOG_CATEGORY_STATIC(LogAVIWriter, Log, All);

class FAVIWriterModule : public IModuleInterface
{
};

IMPLEMENT_MODULE(FAVIWriterModule, AVIWriter);

FCapturedFrame::FCapturedFrame(double InStartTimeSeconds, double InEndTimeSeconds, uint32 InFrameIndex, TArray<FColor> InFrameData)
	: StartTimeSeconds(InStartTimeSeconds)
	, EndTimeSeconds(InEndTimeSeconds)
	, FrameIndex(InFrameIndex)
	, FrameData(MoveTemp(InFrameData))
	, FrameProcessedEvent(nullptr)
{
}

FCapturedFrame::~FCapturedFrame()
{
}

FCapturedFrames::FCapturedFrames(const FString& InArchiveDirectory, int32 InMaxInMemoryFrames)
	: ArchiveDirectory(InArchiveDirectory)
	, MaxInMemoryFrames(InMaxInMemoryFrames)
{
	FrameReady = FPlatformProcess::GetSynchEventFromPool();

	// Ensure the archive directory doesn't exist
	auto& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
	PlatformFile.DeleteDirectoryRecursively(*ArchiveDirectory);

	TotalArchivedFrames = 0;
	InMemoryFrames.Reserve(MaxInMemoryFrames);
}

FCapturedFrames::~FCapturedFrames()
{
	FPlatformProcess::ReturnSynchEventToPool(FrameReady);
	FPlatformFileManager::Get().GetPlatformFile().DeleteDirectoryRecursively(*ArchiveDirectory);
}

void FCapturedFrames::Add(FCapturedFrame Frame)
{
	bool bShouldArchive = false;
	{
		FScopeLock Lock(&ArchiveFrameMutex);
		bShouldArchive = ArchivedFrames.Num() != 0;
	}
	
	if (!bShouldArchive)
	{
		FScopeLock Lock(&InMemoryFrameMutex);
		if (InMemoryFrames.Num() < MaxInMemoryFrames)
		{
			InMemoryFrames.Add(MoveTemp(Frame));
		}
		else
		{
			bShouldArchive = true;
		}
	}

	if (bShouldArchive)
	{
		ArchiveFrame(MoveTemp(Frame));
	}
	else
	{
		FrameReady->Trigger();
	}
}

FArchive &operator<<(FArchive& Ar, FCapturedFrame& Frame)
{
	Ar << Frame.StartTimeSeconds;
	Ar << Frame.EndTimeSeconds;
	Ar << Frame.FrameIndex;
	Ar << Frame.FrameData;
	return Ar;
}

void FCapturedFrames::ArchiveFrame(FCapturedFrame Frame)
{
	auto& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
	if (!PlatformFile.DirectoryExists(*ArchiveDirectory))
	{
		PlatformFile.CreateDirectory(*ArchiveDirectory);
	}

	// Get (and increment) a unique index for this frame
	uint32 ArchivedFrameIndex = ++TotalArchivedFrames;

	FString Filename = ArchiveDirectory / FString::Printf(TEXT("%d.frame"), ArchivedFrameIndex);
	TUniquePtr<FArchive> Archive(IFileManager::Get().CreateFileWriter(*Filename));
	if (ensure(Archive.IsValid()))
	{
		*Archive << Frame;
		Archive->Close();

		// Add the archived frame to the array
		FScopeLock Lock(&ArchiveFrameMutex);
		ArchivedFrames.Add(ArchivedFrameIndex);
	}
}

TOptional<FCapturedFrame> FCapturedFrames::UnArchiveFrame(uint32 FrameIndex) const
{
	FString Filename = ArchiveDirectory / FString::Printf(TEXT("%d.frame"), FrameIndex);
	TUniquePtr<FArchive> Archive(IFileManager::Get().CreateFileReader(*Filename));
	if (ensure(Archive.IsValid()))
	{
		FCapturedFrame Frame;
		*Archive << Frame;
		Archive->Close();

		FPlatformFileManager::Get().GetPlatformFile().DeleteFile(*Filename);
		return MoveTemp(Frame);
	}

	return TOptional<FCapturedFrame>();
}

void FCapturedFrames::StartUnArchiving()
{
	if (UnarchiveTask.IsSet())
	{
		return;
	}

	UnarchiveTask = Async(EAsyncExecution::Thread, [this]{

		// Attempt to unarchive any archived frames
		ArchiveFrameMutex.Lock();
		TArray<uint32> ArchivedFramesToGet = ArchivedFrames;
		ArchiveFrameMutex.Unlock();

		int32 MaxNumToProcess = FMath::Min(ArchivedFramesToGet.Num(), MaxInMemoryFrames);
		for (int32 Index = 0; Index < MaxNumToProcess; ++Index)
		{
			TOptional<FCapturedFrame> Frame = UnArchiveFrame(ArchivedFramesToGet[Index]);

			if (Frame.IsSet())
			{
				FScopeLock Lock(&InMemoryFrameMutex);
				InMemoryFrames.Add(MoveTemp(Frame.GetValue()));
			}
		}

		if (MaxNumToProcess)
		{
			// Only remove the archived frame indices once we have fully processed them (so that FCapturedFrames::Add knows when to archive frames)
			{
				FScopeLock Lock(&ArchiveFrameMutex);
				ArchivedFrames.RemoveAt(0, MaxNumToProcess, false);
			}

			FrameReady->Trigger();
		}
	});
}

TArray<FCapturedFrame> FCapturedFrames::ReadFrames(uint32 WaitTimeMs)
{
	if (!FrameReady->Wait(WaitTimeMs))
	{
		StartUnArchiving();
		return TArray<FCapturedFrame>();
	}

	UnarchiveTask = TOptional<TFuture<void>>();

	TArray<FCapturedFrame> Frames;
	Frames.Reserve(MaxInMemoryFrames);

	// Swap the frames
	{
		FScopeLock Lock(&InMemoryFrameMutex);
		Swap(Frames, InMemoryFrames);
	}

	StartUnArchiving();

	return Frames;
}

int32 FCapturedFrames::GetNumOutstandingFrames() const
{
	int32 TotalNumFrames = 0;
	{
		FScopeLock Lock(&InMemoryFrameMutex);
		TotalNumFrames += InMemoryFrames.Num();
	}

	{
		FScopeLock Lock(&ArchiveFrameMutex);
		TotalNumFrames += ArchivedFrames.Num();
	}
	
	return TotalNumFrames;
}

FAVIWriter::~FAVIWriter()
{
}

void FAVIWriter::Update(double FrameTimeSeconds, TArray<FColor> FrameData)
{
	if (bCapturing && FrameData.Num())
	{
		double FrameLength = double(Options.CaptureFramerateDenominator) / Options.CaptureFramerateNumerator;
		double FrameStart = FrameNumber * FrameLength;
		FCapturedFrame Frame(FrameStart, FrameStart + FrameLength, FrameNumber, MoveTemp(FrameData));

		FEvent* SyncEvent = nullptr;
		if (Options.bSynchronizeFrames)
		{
			SyncEvent = FPlatformProcess::GetSynchEventFromPool();
			Frame.FrameProcessedEvent = SyncEvent;
		}

		// Add the frame
		CapturedFrames->Add(MoveTemp(Frame));
		FrameNumber++;

		if (SyncEvent)
		{
			SyncEvent->Wait(MAX_uint32);
			FPlatformProcess::ReturnSynchEventToPool(SyncEvent);
		}
	}
}

FAVIWriter* FAVIWriter::CreateInstance(const FAVIWriterOptions& InOptions)
{
	return nullptr;
}
