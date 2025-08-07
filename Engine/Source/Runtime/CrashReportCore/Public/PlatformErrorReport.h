// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#if PLATFORM_WINDOWS

#include "Windows/WindowsErrorReport.h"

typedef FWindowsErrorReport FPlatformErrorReport;

#elif PLATFORM_LINUX

#include "Linux/LinuxErrorReport.h"

typedef FLinuxErrorReport FPlatformErrorReport;

#else

typedef FGenericErrorReport FPlatformErrorReport;

#endif // PLATFORM_LINUX
