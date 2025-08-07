// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Physics/ImmediatePhysics/ImmediatePhysicsChaos/ImmediatePhysicsDeclares_Chaos.h"
#include "Physics/ImmediatePhysics/ImmediatePhysicsShared/ImmediatePhysicsCore.h"


/**
 * @file ImmediatePhysicsDeclares.h
 * 
 * Move either ImmediatePhysics_PhysX or ImmediatePhysics_Chaos Immediate Physics into the ImmediatePhysics namespace
 *
 * We can use PhysX ImmediatePhysics only if PhysX is also providing the global physics interface.
 * because ImmediatePhysics_PhysX relies on physx types being instantiated by BodyInstance, ConstraintInstance, etc.
 * 
 * You can currently run PhysX and Chaos ImmediatePhysics simulations in the same build if PhysX is
 * providing the global physics interface. This capability will likely disappear once Chaos is established
 * at which point you will only be able to use Chaos immediate physics if you are also using Chaos
 * for global physics.
 */
namespace ImmediatePhysics
{
	using EActorType = ImmediatePhysics_Shared::EActorType;
	using EForceType = ImmediatePhysics_Shared::EForceType;

#if WITH_CHAOS
	using FSimulation = ImmediatePhysics_Chaos::FSimulation;
	using FActorHandle = ImmediatePhysics_Chaos::FActorHandle;
	using FJointHandle = ImmediatePhysics_Chaos::FJointHandle;
#else
#error Global ImmediatePhysics is not defined
#endif
}