// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "PhysicsInterfaceDeclares.h"
#include "PhysicsCore.h"


#if WITH_CHAOS

#include "Physics/Experimental/PhysInterface_Chaos.h"
#include "Physics/Experimental/PhysScene_Chaos.h"

#else

static_assert(false, "A physics engine interface must be defined to build");

#endif
