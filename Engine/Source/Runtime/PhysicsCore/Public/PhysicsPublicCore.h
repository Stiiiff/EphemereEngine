// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

class UPhysicalMaterial;

/** Set of delegates to allowing hooking different parts of the physics engine */
class PHYSICSCORE_API FPhysicsDelegatesCore
{
public:
	DECLARE_MULTICAST_DELEGATE_OneParam(FOnUpdatePhysXMaterial, UPhysicalMaterial*);
	static FOnUpdatePhysXMaterial OnUpdatePhysXMaterial;
};


#if WITH_PHYSX
namespace physx
{
	class PxPhysics;
	class PxMaterial;
}

/** Pointer to PhysX SDK object */
extern PHYSICSCORE_API physx::PxPhysics* GPhysXSDK;
/** Pointer to PhysX allocator */
extern PHYSICSCORE_API class FPhysXAllocator* GPhysXAllocator;

extern PHYSICSCORE_API class IPhysXCookingModule* GetPhysXCookingModule(bool bForceLoad = true);

/** Array of PxMaterial objects which are awaiting cleaning up. */
extern PHYSICSCORE_API TArray<physx::PxMaterial*> GPhysXPendingKillMaterial;

#endif