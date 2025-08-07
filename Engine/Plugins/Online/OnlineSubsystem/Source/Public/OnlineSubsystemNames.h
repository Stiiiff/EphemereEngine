// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#define OSS_PLATFORM_NAME_WINDOWS	TEXT("WIN")
#define OSS_PLATFORM_NAME_LINUX		TEXT("LNX")
#define OSS_PLATFORM_NAME_OTHER		TEXT("OTHER")

#ifndef NULL_SUBSYSTEM
#define NULL_SUBSYSTEM FName(TEXT("NULL"))
#endif

#ifndef STEAM_SUBSYSTEM
#define STEAM_SUBSYSTEM FName(TEXT("STEAM"))
#endif

#ifndef MCP_SUBSYSTEM
#define MCP_SUBSYSTEM FName(TEXT("MCP"))
#endif

#ifndef MCP_SUBSYSTEM_EMBEDDED
#define MCP_SUBSYSTEM_EMBEDDED FName(TEXT("MCP:EMBEDDED"))
#endif
