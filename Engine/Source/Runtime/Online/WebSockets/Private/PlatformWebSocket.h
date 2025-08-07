// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "HAL/Platform.h"

#if WITH_LIBWEBSOCKETS
	#include "Lws/LwsWebSocketsManager.h"
#elif WITH_WINHTTPWEBSOCKETS
	#include "WinHttp/WinHttpWebSocketsManager.h"
#else
	#error "Web Sockets not implemented on this platform yet"
#endif

#if WITH_LIBWEBSOCKETS
	typedef FLwsWebSocketsManager FPlatformWebSocketsManager;
#elif WITH_WINHTTPWEBSOCKETS
	typedef FWinHttpWebSocketsManager FPlatformWebSocketsManager;
#endif
