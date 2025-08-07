// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#ifndef WANTS_MRMESH_UVS
	#if PLATFORM_WINDOWS
		#define WANTS_MRMESH_UVS 1
	#endif
#endif

#ifndef WANTS_MRMESH_TANGENTS
	#if PLATFORM_WINDOWS
		#define WANTS_MRMESH_TANGENTS 1
	#endif
#endif

#ifndef WANTS_MRMESH_COLORS
	#if PLATFORM_WINDOWS
		#define WANTS_MRMESH_COLORS 1
	#endif
#endif

#ifndef MRMESH_INDEX_TYPE
	#define MRMESH_INDEX_TYPE uint32
#endif
