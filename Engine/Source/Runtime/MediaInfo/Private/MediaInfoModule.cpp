// Copyright Epic Games, Inc. All Rights Reserved.

#include "CoreTypes.h"

#include "Modules/ModuleManager.h"
#include "IMediaInfoModule.h"
#include "IMediaModule.h"
#include "Misc/Guid.h"

#define LOCTEXT_NAMESPACE "MediaInfoModule"

// ----------------------------------------------------------------------------------------------

class FMediaInfoModule : public IMediaInfoModule
{
public:
	void StartupModule() override
	{
	}

	void Initialize(IMediaModule* MediaModule) override
	{
		MediaModule->RegisterPlatform(TEXT("Windows"), FGuid(0xd1d5f296, 0xff834a87, 0xb20faaa9, 0xd6b8e9a6), this);
		MediaModule->RegisterPlatform(TEXT("Linux"), FGuid(0x115de4fe, 0x241b465b, 0x970a872f, 0x3167492a), this);
		MediaModule->RegisterPlatform(TEXT("Unix"), FGuid(0xa15b98db, 0x84ca4b5a, 0x84ababff, 0xb9d552f3), this);
	}
};

IMPLEMENT_MODULE(FMediaInfoModule, MediaInfo);

#undef LOCTEXT_NAMESPACE
