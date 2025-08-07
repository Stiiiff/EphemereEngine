// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#if PLATFORM_WINDOWS
#include "Windows/AllowWindowsPlatformTypes.h"

// D3D headers.


// Disable macro redefinition warning for compatibility with Windows SDK 8+
#pragma warning(push)
#pragma warning(disable : 4005)	// macro redefinition

#include <d3d11.h>
#include <d3d11_1.h>
#include <D3Dcompiler.h>

#pragma warning(pop)

#include "Windows/HideWindowsPlatformTypes.h"
#endif		// PLATFORM_WINDOWS

#if PLATFORM_WINDOWS
#include "Windows/AllowWindowsPlatformTypes.h"
#include <GL/gl.h>
#include <GL/glext.h>
#include <GL/wglext.h>
#include "Windows/HideWindowsPlatformTypes.h"
#elif PLATFORM_LINUX
//#define GLCOREARB_PROTOTYPES 
#include <GL/glcorearb.h>
//#include <GL/glext.h>
#include "SDL.h"

#endif // PLATFORM_LINUX
