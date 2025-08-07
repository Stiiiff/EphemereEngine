// Copyright Epic Games, Inc. All Rights Reserved.

#include "FileMediaSource.h"
#include "Misc/Paths.h"
#include "HAL/FileManager.h"
#include "Misc/PackageName.h"

namespace FileMediaSource
{
	/** Name of the PrecacheFile media option. */
	static const FName PrecacheFileOption("PrecacheFile");
}


/* UFileMediaSource interface
 *****************************************************************************/

FString UFileMediaSource::GetFullPath() const
{
	ResolveFullPath();
	return ResolvedFullPath;
}


void UFileMediaSource::SetFilePath(const FString& Path)
{
	ClearResolvedFullPath();
	if (Path.IsEmpty() || Path.StartsWith(TEXT("./")))
	{
		FilePath = Path;
	}
	else
	{
		FString FullPath = FPaths::ConvertRelativePathToFull(Path);
		const FString FullGameContentDir = FPaths::ConvertRelativePathToFull(FPaths::ProjectContentDir());

		if (FullPath.StartsWith(FullGameContentDir))
		{
			FPaths::MakePathRelativeTo(FullPath, *FullGameContentDir);
			FullPath = FString(TEXT("./")) + FullPath;
		}

		FilePath = FullPath;
	}
}

#if WITH_EDITOR
void UFileMediaSource::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	if (PropertyChangedEvent.GetPropertyName() == GET_MEMBER_NAME_CHECKED(UFileMediaSource, FilePath))
	{
		ClearResolvedFullPath();
	}
}
#endif

void UFileMediaSource::ClearResolvedFullPath() const
{
	ResolvedFullPath.Empty();
}

bool UFileMediaSource::TryResolveRelativeToProjectDir() const
{
	if (ResolvedFullPath.StartsWith(TEXT("./")))
	{
		ResolvedFullPath = FPaths::Combine(FPaths::ProjectContentDir(), FilePath.RightChop(2));
		FString FinalFullPath = FPaths::ConvertRelativePathToFull(ResolvedFullPath);
		if (FPaths::FileExists(FinalFullPath))
		{
			ResolvedFullPath = FinalFullPath;
			return true;
		}
	}

	return false;
}

bool UFileMediaSource::TryResolveRelativeToRootDirectories() const
{
	const TArray<FString>& RootDirectories = FPlatformMisc::GetAdditionalRootDirectories();
	FString RelativeToRootPath = ResolvedFullPath;

	if (RelativeToRootPath.StartsWith(TEXT("./")))
	{
		RelativeToRootPath = FPaths::Combine(FPaths::ProjectContentDir(), FilePath.RightChop(2));
	}

	if (RelativeToRootPath.StartsWith(FPaths::GetRelativePathToRoot()))
	{
		// if we start with the relative path to root then remove that so we can change the root directory below
		RelativeToRootPath = RelativeToRootPath.Mid(FPaths::GetRelativePathToRoot().Len());
	}
	for (const FString& RootPath : RootDirectories)
	{
		FString FinalFullPath = FPaths::ConvertRelativePathToFull(FPaths::Combine(RootPath, RelativeToRootPath));
		if (FPaths::FileExists(FinalFullPath))
		{
			ResolvedFullPath = FinalFullPath;

			return true;
		}
	}

	return false;
}

void UFileMediaSource::ResolveFullPath() const
{
	if (!ResolvedFullPath.IsEmpty())
	{
		return;
	}

	ResolvedFullPath = FilePath;// prevent reentry on the fail case

	if (!FPaths::IsRelative(FilePath))
	{
		return;
	}
    
	if (TryResolveRelativeToRootDirectories())
	{
		return;
	}

	if (TryResolveRelativeToProjectDir())
	{
		return;
	}
    
}

/* IMediaSource overrides
 *****************************************************************************/

bool UFileMediaSource::GetMediaOption(const FName& Key, bool DefaultValue) const
{
	if (Key == FileMediaSource::PrecacheFileOption)
	{
		return PrecacheFile;
	}

	return Super::GetMediaOption(Key, DefaultValue);
}


bool UFileMediaSource::HasMediaOption(const FName& Key) const
{
	if (Key == FileMediaSource::PrecacheFileOption)
	{
		return true;
	}

	return Super::HasMediaOption(Key);
}

/* UMediaSource overrides
 *****************************************************************************/

FString UFileMediaSource::GetUrl() const
{
	return FString(TEXT("file://")) + GetFullPath();
}

bool UFileMediaSource::Validate() const
{
	ResolveFullPath();

	check( (ResolvedFullPath.IsEmpty() == false) || FilePath.IsEmpty() );

	return FPaths::FileExists(ResolvedFullPath);
}
