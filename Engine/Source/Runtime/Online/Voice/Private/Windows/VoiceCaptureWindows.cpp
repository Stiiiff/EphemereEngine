// Copyright Epic Games, Inc. All Rights Reserved.

#include "VoiceCaptureWindows.h"
#include "VoicePrivate.h"
#include "VoiceModule.h"
#include "DSP/Dsp.h"

#if PLATFORM_SUPPORTS_VOICE_CAPTURE

#include "Windows/AllowWindowsPlatformTypes.h"

static int32 DisplayAmplitudeCvar = 0;
FAutoConsoleVariableRef CVarDisplayAmplitude(
	TEXT("voice.debug.PrintAmplitude"),
	DisplayAmplitudeCvar,
	TEXT("when set to 1, the current incoming amplitude of the VOIP engine will be displayed on screen.\n")
	TEXT("0: disabled, 1: enabled."),
	ECVF_Default);

struct FVoiceCaptureWindowsVars
{
	/** GUID of current voice capture device */
	GUID VoiceCaptureDeviceGuid;
	/** Voice capture device caps */
	DSCCAPS VoiceCaptureDevCaps;
	/** Wave format of buffer */
	WAVEFORMATEX WavFormat;
	/** Buffer description */
	DSCBUFFERDESC VoiceCaptureBufferDesc;
	/** Buffer caps */
	DSCBCAPS VoiceCaptureBufferCaps8;
	/** Notification events */
	HANDLE StopEvent;
	/** Current audio position of valid data in capture buffer */
	DWORD NextCaptureOffset;

	FVoiceCaptureWindowsVars() :
		NextCaptureOffset(0)
	{
		StopEvent = INVALID_HANDLE_VALUE;
		Reset();
	}

	void Reset()
	{
		if (StopEvent != INVALID_HANDLE_VALUE)
		{
			CloseHandle(StopEvent);
			StopEvent = INVALID_HANDLE_VALUE;
		}

		NextCaptureOffset = 0;

		FMemory::Memzero(&VoiceCaptureDeviceGuid, sizeof(GUID));
		FMemory::Memzero(&VoiceCaptureDevCaps, sizeof(VoiceCaptureDevCaps));

		FMemory::Memzero(&WavFormat, sizeof(WavFormat));
		FMemory::Memzero(&VoiceCaptureBufferDesc, sizeof(VoiceCaptureBufferDesc));
		FMemory::Memzero(&VoiceCaptureBufferCaps8, sizeof(VoiceCaptureBufferCaps8));
	}
};

FVoiceCaptureWindows::FVoiceCaptureWindows() :
	CV(nullptr),
	LastCaptureTime(0.0),
	VoiceCaptureState(EVoiceCaptureState::UnInitialized)
{
	CV = new FVoiceCaptureWindowsVars();
}

FVoiceCaptureWindows::~FVoiceCaptureWindows()
{
	Shutdown();

	FVoiceCaptureDeviceWindows* VoiceCaptureDev = FVoiceCaptureDeviceWindows::Get();
 	if (VoiceCaptureDev)	
	{
 		VoiceCaptureDev->FreeVoiceCaptureObject(this);
 	}
	
	if (CV)
	{
		delete CV;
		CV = nullptr;
	}	
}

bool FVoiceCaptureWindows::Init(const FString& DeviceName, int32 SampleRate, int32 NumChannels)
{
	FVoiceCaptureDeviceWindows* VoiceDev = FVoiceCaptureDeviceWindows::Get();
	if (!VoiceDev)
	{
		UE_LOG(LogVoiceCapture, Warning, TEXT("No voice capture interface."));
		return false;
	}
	
   	// init the sample counter to 0 on init
	SampleCounter = 0; 
	CachedSampleStart = 0;

	// set up level detector
	static IConsoleVariable* SilenceDetectionAttackCVar = IConsoleManager::Get().FindConsoleVariable(TEXT("voice.SilenceDetectionAttackTime"));
	check(SilenceDetectionAttackCVar);			
	static IConsoleVariable* SilenceDetectionReleaseCVar = IConsoleManager::Get().FindConsoleVariable(TEXT("voice.SilenceDetectionReleaseTime"));
	check(SilenceDetectionReleaseCVar);				

	MicLevelDetector.Init(SampleRate, SilenceDetectionAttackCVar->GetFloat(), SilenceDetectionReleaseCVar->GetFloat(), MicSilenceDetectionConfig::LevelDetectionMode, MicSilenceDetectionConfig::IsAnalog);
	
	const int32 AttackInSamples = SampleRate * SilenceDetectionAttackCVar->GetFloat() * 0.001f;
	LookaheadBuffer.Init(AttackInSamples + 1);
	LookaheadBuffer.SetDelay(AttackInSamples);

	NoiseGateAttenuator.Init(SampleRate);

	bIsMicActive = false;
	bWasMicAboveNoiseGateThreshold = false;

	return CreateCaptureBuffer(DeviceName.IsEmpty() ? VoiceDev->DefaultVoiceCaptureDevice.DeviceName : DeviceName, SampleRate, NumChannels);
}

bool FVoiceCaptureWindows::CreateCaptureBuffer(const FString& DeviceName, int32 SampleRate, int32 NumChannels)
{
	// Free the previous buffer
	FreeCaptureBuffer();
	VoiceCaptureState = EVoiceCaptureState::NotCapturing;

	if (SampleRate < 8000 || SampleRate > 48000)
	{
		UE_LOG(LogVoiceCapture, Warning, TEXT("Voice capture doesn't support %d hz"), SampleRate);
		return false;
	}

	if (NumChannels < 0 || NumChannels > 2)
	{
		UE_LOG(LogVoiceCapture, Warning, TEXT("Voice capture only supports 1 or 2 channels"));
		return false;
	}

	FVoiceCaptureDeviceWindows* VoiceDev = FVoiceCaptureDeviceWindows::Get();
	if (!VoiceDev)
	{
		UE_LOG(LogVoiceCapture, Warning, TEXT("No voice capture interface."));
		return false;
	}

	FVoiceCaptureDeviceWindows::FCaptureDeviceInfo* DeviceInfo = nullptr;
	if (DeviceName.IsEmpty())
	{
		DeviceInfo = &VoiceDev->DefaultVoiceCaptureDevice;
	}
	else
	{
		DeviceInfo = VoiceDev->Devices.Find(DeviceName);
	}

	if (DeviceInfo)
	{
		UE_LOG(LogVoiceCapture, Display, TEXT("Creating capture %s [%d:%d]"), *DeviceInfo->DeviceName, SampleRate, NumChannels);
		CV->VoiceCaptureDeviceGuid = DeviceInfo->DeviceId;

		UE_LOG(LogVoiceCapture, Warning, TEXT("Failed to create capture device"));
		return false;
	}
	else
	{
		UE_LOG(LogVoiceCapture, Warning, TEXT("No voice capture device %s found."), *DeviceName);
		return false;
	}

	UncompressedAudioBuffer.Init(0, CV->VoiceCaptureBufferDesc.dwBufferBytes);
	check(UncompressedAudioBuffer.Max() >= (int32)CV->VoiceCaptureBufferCaps8.dwBufferBytes);

	NumInputChannels = CV->WavFormat.nChannels;

	ReleaseBuffer.Init((int32)CV->VoiceCaptureBufferCaps8.dwBufferBytes);
	ReleaseBuffer.SetDelay(1);
	return true;
}

void FVoiceCaptureWindows::FreeCaptureBuffer()
{
	// Stop playback
	Stop();

	// Release all D3D8 resources
	CV->Reset();

	VoiceCaptureState = EVoiceCaptureState::UnInitialized;
}

void FVoiceCaptureWindows::Shutdown()
{
	FreeCaptureBuffer();
}

bool FVoiceCaptureWindows::Start()
{
	check(VoiceCaptureState != EVoiceCaptureState::UnInitialized);

	UE_LOG(LogVoiceCapture, Warning, TEXT("CV->VoiceCaptureBuffer8 == nullptr"));
	return false;
}

void FVoiceCaptureWindows::Stop()
{

}

bool FVoiceCaptureWindows::ChangeDevice(const FString& DeviceName, int32 SampleRate, int32 NumChannels)
{
	if (VoiceCaptureState != EVoiceCaptureState::UnInitialized)
	{
		return CreateCaptureBuffer(DeviceName, SampleRate, NumChannels);
	}
	else
	{
		UE_LOG(LogVoiceCapture, Warning, TEXT("Unable to change device, not initialized"));
		return false;
	}
}

bool FVoiceCaptureWindows::IsCapturing()
{		
	return false;
}

EVoiceCaptureState::Type FVoiceCaptureWindows::GetCaptureState(uint32& OutAvailableVoiceData) const
{
	if (VoiceCaptureState != EVoiceCaptureState::UnInitialized &&
		VoiceCaptureState != EVoiceCaptureState::Error)
	{
		OutAvailableVoiceData = UncompressedAudioBuffer.Num();
	}
	else
	{
		OutAvailableVoiceData = 0;
	}

	return VoiceCaptureState;
}

void FVoiceCaptureWindows::ProcessData()
{
	DWORD CurrentCapturePos = 0;
	DWORD CurrentReadPos = 0;

	HRESULT hr = E_FAIL;
	if (FAILED(hr))
	{
		UE_LOG(LogVoiceCapture, Warning, TEXT("Failed to get voice buffer cursor position 0x%08x"), hr);
		VoiceCaptureState = EVoiceCaptureState::Error;
		return;
	}

	DWORD LockSize = ((CurrentReadPos - CV->NextCaptureOffset) + CV->VoiceCaptureBufferCaps8.dwBufferBytes) % CV->VoiceCaptureBufferCaps8.dwBufferBytes;
	if(LockSize != 0) 
	{ 
		//UE_LOG( LogVoiceCapture, Log, TEXT( "LockSize: %i, CurrentCapturePos: %i, CurrentReadPos: %i, NextCaptureOffset: %i" ), LockSize, CurrentCapturePos, CurrentReadPos, CV->NextCaptureOffset );

		DWORD CaptureFlags = 0;
		DWORD CaptureLength = 0;
		void* CaptureData = nullptr;
		DWORD CaptureLength2 = 0;
		void* CaptureData2 = nullptr;
		
		UE_LOG(LogVoiceCapture, Warning, TEXT("Failed to lock voice buffer"));
		VoiceCaptureState = EVoiceCaptureState::Error;
	}
}

EVoiceCaptureState::Type FVoiceCaptureWindows::GetVoiceData(uint8* OutVoiceBuffer, const uint32 InVoiceBufferSize, uint32& OutBytesWritten, uint64& OutSampleClockCounter)
{
	EVoiceCaptureState::Type NewMicState = VoiceCaptureState;
	OutBytesWritten = 0;

	if (VoiceCaptureState == EVoiceCaptureState::Ok ||
		VoiceCaptureState == EVoiceCaptureState::Stopping)
	{

		if (InVoiceBufferSize >= (uint32) UncompressedAudioBuffer.Num())
		{
			OutBytesWritten = UncompressedAudioBuffer.Num();
			FMemory::Memcpy(OutVoiceBuffer, UncompressedAudioBuffer.GetData(), OutBytesWritten);
			VoiceCaptureState = EVoiceCaptureState::NoData;
			UncompressedAudioBuffer.Reset();

			OutSampleClockCounter = CurrentSampleStart;
		}
		else
		{
			NewMicState = EVoiceCaptureState::BufferTooSmall;
		}
	}

	// If we have any sends for this microphones output, push to them here.
	if (MicrophoneOutput.Num() > 0)
	{
		// Convert our buffer from int16 to float:
		int16* OutputData = reinterpret_cast<int16*>(OutVoiceBuffer);
		uint32 NumSamples = OutBytesWritten / sizeof(int16);
		ConversionBuffer.Reset();
		// Note: Sample rate is unused for this operation.
		ConversionBuffer.Append(OutputData, NumSamples, NumInputChannels, 16000);
		
		if (NumInputChannels > 1)
		{
			// For consistency, mixdown to mono.
			ConversionBuffer.MixBufferToChannels(1);
		}

		MicrophoneOutput.PushAudio(ConversionBuffer.GetData(), ConversionBuffer.GetNumSamples());
	}

	// print debug string with current amplitude:
	if (DisplayAmplitudeCvar && GEngine)
	{
		static double TimeLastPrinted = FPlatformTime::Seconds();

		static const double AmplitudeStringDisplayRate = 0.05;
		static const int32 TotalNumTicks = 32;

		if (FPlatformTime::Seconds() - TimeLastPrinted > AmplitudeStringDisplayRate)
		{
			const float MicLevel = MicLevelDetector.GetCurrentValue();
			FString PrintString = FString::Printf(TEXT("Mic Amp: %.2f"), MicLevel);

			int32 NumTicks = FMath::FloorToInt(MicLevelDetector.GetCurrentValue() * TotalNumTicks);

			for (int32 Iteration = 0; Iteration < NumTicks; Iteration++)
			{
				PrintString.AppendChar(TCHAR('|'));
			}

			FColor TextColor = FLinearColor::LerpUsingHSV(FLinearColor::Green, FLinearColor::Red, MicLevel).ToFColor(true);

			GEngine->AddOnScreenDebugMessage(30, AmplitudeStringDisplayRate, TextColor, PrintString, false);
			TimeLastPrinted = FPlatformTime::Seconds();
		}
	}

	return NewMicState;
}

EVoiceCaptureState::Type FVoiceCaptureWindows::GetVoiceData(uint8* OutVoiceBuffer, uint32 InVoiceBufferSize, uint32& OutAvailableVoiceData)
{
	uint64 UnusedSampleCounter = 0;
	return GetVoiceData(OutVoiceBuffer, InVoiceBufferSize, OutAvailableVoiceData, UnusedSampleCounter);
}

int32 FVoiceCaptureWindows::GetBufferSize() const
{
	if (VoiceCaptureState != EVoiceCaptureState::UnInitialized)
	{
		return CV->VoiceCaptureBufferCaps8.dwBufferBytes;
	}

	return 0;
}

bool FVoiceCaptureWindows::CreateNotifications(uint32 BufferSize)
{
	bool bSuccess = false;
	
	UE_LOG(LogVoiceCapture, Warning, TEXT("Failed to create voice notification interface"));

	return bSuccess;
}

bool FVoiceCaptureWindows::Tick(float DeltaTime)
{
    QUICK_SCOPE_CYCLE_COUNTER(STAT_FVoiceCaptureWindows_Tick);

	if (VoiceCaptureState != EVoiceCaptureState::UnInitialized &&
		VoiceCaptureState != EVoiceCaptureState::NotCapturing)
	{
		ProcessData();

		if (CV->StopEvent != INVALID_HANDLE_VALUE && WaitForSingleObject(CV->StopEvent, 0) == WAIT_OBJECT_0)
		{
			UE_LOG(LogVoiceCapture, Verbose, TEXT("Voice capture stopped"));
			ResetEvent(CV->StopEvent);
			VoiceCaptureState = EVoiceCaptureState::NotCapturing;
			UncompressedAudioBuffer.Empty(UncompressedAudioBuffer.Max());
		}
	}

	return true;
}

float FVoiceCaptureWindows::GetCurrentAmplitude() const
{
	return MicLevelDetector.GetCurrentValue();
}

void FVoiceCaptureWindows::DumpState() const
{
#if !NO_LOGGING
	if (CV)
	{
		extern FString PrintMSGUID(LPGUID Guid);
		UE_LOG(LogVoiceCapture, Display, TEXT("Device %s"), *PrintMSGUID(&CV->VoiceCaptureDeviceGuid));

		UE_LOG(LogVoiceCapture, Display, TEXT("Capture Format"));
		UE_LOG(LogVoiceCapture, Display, TEXT("- Tag: %d"), CV->WavFormat.wFormatTag);
		UE_LOG(LogVoiceCapture, Display, TEXT("- Channels: %d"), CV->WavFormat.nChannels);
		UE_LOG(LogVoiceCapture, Display, TEXT("- BitsPerSample: %d"), CV->WavFormat.wBitsPerSample);
		UE_LOG(LogVoiceCapture, Display, TEXT("- SamplesPerSec: %d"), CV->WavFormat.nSamplesPerSec);
		UE_LOG(LogVoiceCapture, Display, TEXT("- BlockAlign: %d"), CV->WavFormat.nBlockAlign);
		UE_LOG(LogVoiceCapture, Display, TEXT("- AvgBytesPerSec: %d"), CV->WavFormat.nAvgBytesPerSec);

		UE_LOG(LogVoiceCapture, Display, TEXT("Capture Buffer"));
		UE_LOG(LogVoiceCapture, Display, TEXT("- Flags: 0x%08x"), CV->VoiceCaptureBufferDesc.dwFlags);
		UE_LOG(LogVoiceCapture, Display, TEXT("- BufferBytes: %d"), CV->VoiceCaptureBufferDesc.dwBufferBytes);
		UE_LOG(LogVoiceCapture, Display, TEXT("- Format: 0x%08x"), CV->VoiceCaptureBufferDesc.lpwfxFormat);

		UE_LOG(LogVoiceCapture, Display, TEXT("Device Caps"));
		UE_LOG(LogVoiceCapture, Display, TEXT("- Size: %d"), CV->VoiceCaptureDevCaps.dwSize);
		UE_LOG(LogVoiceCapture, Display, TEXT("- Flags: 0x%08x"), CV->VoiceCaptureDevCaps.dwFlags);
		UE_LOG(LogVoiceCapture, Display, TEXT("- Formats: %d"), CV->VoiceCaptureDevCaps.dwFormats);
		UE_LOG(LogVoiceCapture, Display, TEXT("- Channels: %d"), CV->VoiceCaptureDevCaps.dwChannels);

		UE_LOG(LogVoiceCapture, Display, TEXT("D3D8 Caps"));
		UE_LOG(LogVoiceCapture, Display, TEXT("- Size: %d"), CV->VoiceCaptureBufferCaps8.dwSize);
		UE_LOG(LogVoiceCapture, Display, TEXT("- Flags: 0x%08x"), CV->VoiceCaptureBufferCaps8.dwFlags);
		UE_LOG(LogVoiceCapture, Display, TEXT("- BufferBytes: %d"), CV->VoiceCaptureBufferCaps8.dwBufferBytes);
	}
	else
	{
		UE_LOG(LogVoiceCapture, Display, TEXT("No capture device to dump state"));
	}
#endif // !NO_LOGGING
}

#include "Windows/HideWindowsPlatformTypes.h"

#endif // PLATFORM_SUPPORTS_VOICE_CAPTURE
