// Copyright Epic Games, Inc. All Rights Reserved.

#include "PhysicsEngine/PhysSubstepTasks.h"
#include "PhysicsEngine/PhysicsSettings.h"

struct FSubstepCallbackGuard
{
#if !UE_BUILD_SHIPPING
	FSubstepCallbackGuard(FPhysSubstepTask& InSubstepTask) : SubstepTask(InSubstepTask)
	{
		++SubstepTask.SubstepCallbackGuard;
	}

	~FSubstepCallbackGuard()
	{
		--SubstepTask.SubstepCallbackGuard;
	}

	FPhysSubstepTask& SubstepTask;
#else
	FSubstepCallbackGuard(FPhysSubstepTask&)
	{
	}
#endif
};

void FPhysSubstepTask::SwapBuffers()
{
	External = !External;
}

void FPhysSubstepTask::RemoveBodyInstance_AssumesLocked(FBodyInstance* BodyInstance)
{
	PhysTargetBuffers[External].Remove(BodyInstance);
	PhysTargetBuffers[!External].Remove(BodyInstance);
}

void FPhysSubstepTask::SetKinematicTarget_AssumesLocked(FBodyInstance* Body, const FTransform& TM)
{
#if WITH_PHYSX
	TM.DiagnosticCheck_IsValid();

	//We only interpolate kinematic actors that need it
	if (!Body->IsNonKinematic() && Body->ShouldInterpolateWhenSubStepping())
	{
		FKinematicTarget_AssumesLocked KinmaticTarget(Body, TM);
		FPhysTarget & TargetState = PhysTargetBuffers[External].FindOrAdd(Body);
		TargetState.bKinematicTarget = true;
		TargetState.KinematicTarget = KinmaticTarget;
	}
#endif
}

bool FPhysSubstepTask::GetKinematicTarget_AssumesLocked(const FBodyInstance* Body, FTransform& OutTM) const
{
#if WITH_PHYSX
	if (const FPhysTarget* TargetState = PhysTargetBuffers[External].Find(Body))
	{
		if (TargetState->bKinematicTarget)
		{
			OutTM = TargetState->KinematicTarget.TargetTM;
			return true;
		}
	}
#endif

	return false;
}

void FPhysSubstepTask::AddCustomPhysics_AssumesLocked(FBodyInstance* Body, const FCalculateCustomPhysics& CalculateCustomPhysics)
{
#if WITH_PHYSX
	//Limit custom physics to non kinematic actors
	if (Body->IsNonKinematic())
	{
		FCustomTarget CustomTarget(CalculateCustomPhysics);

		FPhysTarget & TargetState = PhysTargetBuffers[External].FindOrAdd(Body);
		TargetState.CustomPhysics.Add(CustomTarget);
	}
#endif
}

#if !UE_BUILD_SHIPPING
#define SUBSTEPPING_WARNING() ensureMsgf(SubstepCallbackGuard == 0, TEXT("Applying a sub-stepped force from a substep callback. This usually indicates an error. Make sure you're only using physx data, and that you are adding non-substepped forces"));
#else
#define SUBSTEPPING_WARNING()
#endif

void FPhysSubstepTask::AddForce_AssumesLocked(FBodyInstance* Body, const FVector& Force, bool bAccelChange)
{
#if WITH_PHYSX
	//We should only apply forces on non kinematic actors
	if (Body->IsNonKinematic())
	{
		SUBSTEPPING_WARNING()
		FForceTarget ForceTarget;
		ForceTarget.bPosition = false;
		ForceTarget.Force = Force;
		ForceTarget.bAccelChange = bAccelChange;

		FPhysTarget & TargetState = PhysTargetBuffers[External].FindOrAdd(Body);
		TargetState.Forces.Add(ForceTarget);
	}
#endif
}

void FPhysSubstepTask::AddForceAtPosition_AssumesLocked(FBodyInstance* Body, const FVector& Force, const FVector& Position, bool bIsLocalForce)
{
#if WITH_PHYSX
	if (Body->IsNonKinematic())
	{
		SUBSTEPPING_WARNING()
		FForceTarget ForceTarget;
		ForceTarget.bPosition = true;
		ForceTarget.Force = Force;
		ForceTarget.Position = Position;
		ForceTarget.bIsLocalForce = bIsLocalForce;

		FPhysTarget & TargetState = PhysTargetBuffers[External].FindOrAdd(Body);
		TargetState.Forces.Add(ForceTarget);
	}
#endif
}
void FPhysSubstepTask::AddTorque_AssumesLocked(FBodyInstance* Body, const FVector& Torque, bool bAccelChange)
{
#if WITH_PHYSX
	//We should only apply torque on non kinematic actors
	if (Body->IsNonKinematic())
	{
		SUBSTEPPING_WARNING()
		FTorqueTarget TorqueTarget;
		TorqueTarget.Torque = Torque;
		TorqueTarget.bAccelChange = bAccelChange;

		FPhysTarget & TargetState = PhysTargetBuffers[External].FindOrAdd(Body);
		TargetState.Torques.Add(TorqueTarget);
	}
#endif
}

void FPhysSubstepTask::ClearTorques_AssumesLocked(FBodyInstance* Body)
{
#if WITH_PHYSX
	if (Body->IsNonKinematic())
	{
		SUBSTEPPING_WARNING()
		FPhysTarget & TargetState = PhysTargetBuffers[External].FindOrAdd(Body);
		TargetState.Torques.Empty();
	}
#endif
}

void FPhysSubstepTask::AddRadialForceToBody_AssumesLocked(FBodyInstance* Body, const FVector& Origin, const float Radius, const float Strength, const uint8 Falloff, const bool bAccelChange)
{
#if WITH_PHYSX
	//We should only apply torque on non kinematic actors
	if (Body->IsNonKinematic())
	{
		SUBSTEPPING_WARNING()
		FRadialForceTarget RadialForceTarget;
		RadialForceTarget.Origin = Origin;
		RadialForceTarget.Radius = Radius;
		RadialForceTarget.Strength = Strength;
		RadialForceTarget.Falloff = Falloff;
		RadialForceTarget.bAccelChange = bAccelChange;

		FPhysTarget & TargetState = PhysTargetBuffers[External].FindOrAdd(Body);
		TargetState.RadialForces.Add(RadialForceTarget);
	}
#endif
}

void FPhysSubstepTask::ClearForces_AssumesLocked(FBodyInstance* Body)
{
#if WITH_PHYSX
	if (Body->IsNonKinematic())
	{
		SUBSTEPPING_WARNING()
		FPhysTarget & TargetState = PhysTargetBuffers[External].FindOrAdd(Body);
		TargetState.Forces.Empty();
		TargetState.RadialForces.Empty();
	}
#endif
}

/** Applies custom physics - Assumes caller has obtained writer lock */
void FPhysSubstepTask::ApplyCustomPhysics(const FPhysTarget& PhysTarget, FBodyInstance* BodyInstance, float DeltaTime)
{
#if WITH_PHYSX
	FSubstepCallbackGuard Guard(*this);
	for (int32 i = 0; i < PhysTarget.CustomPhysics.Num(); ++i)
	{
		const FCustomTarget& CustomTarget = PhysTarget.CustomPhysics[i];

		CustomTarget.CalculateCustomPhysics->ExecuteIfBound(DeltaTime, BodyInstance);
	}
#endif
}

/** Applies forces - Assumes caller has obtained writer lock */
void FPhysSubstepTask::ApplyForces_AssumesLocked(const FPhysTarget& PhysTarget, FBodyInstance* BodyInstance)
{
#if WITH_PHYSX
#if WITH_CHAOS
    check(false);
#else
	/** Apply Forces */
	PxRigidBody* PRigidBody = FPhysicsInterface::GetPxRigidBody_AssumesLocked(BodyInstance->GetPhysicsActorHandle());

	for (int32 i = 0; i < PhysTarget.Forces.Num(); ++i)
	{
		const FForceTarget& ForceTarget = PhysTarget.Forces[i];

		if (ForceTarget.bPosition)
		{
			if (!ForceTarget.bIsLocalForce)
			{
				PxRigidBodyExt::addForceAtPos(*PRigidBody, U2PVector(ForceTarget.Force), U2PVector(ForceTarget.Position), PxForceMode::eFORCE, true);
			}
			else
			{
				PxRigidBodyExt::addLocalForceAtLocalPos(*PRigidBody, U2PVector(ForceTarget.Force), U2PVector(ForceTarget.Position), PxForceMode::eFORCE, true);
			}
		}
		else
		{
			PRigidBody->addForce(U2PVector(ForceTarget.Force), ForceTarget.bAccelChange ? PxForceMode::eACCELERATION : PxForceMode::eFORCE, true);
		}
	}
#endif
#endif
}

/** Applies torques - Assumes caller has obtained writer lock */
void FPhysSubstepTask::ApplyTorques_AssumesLocked(const FPhysTarget& PhysTarget, FBodyInstance* BodyInstance)
{
#if WITH_PHYSX
#if WITH_CHAOS
    check(false);
#else
	/** Apply Torques */
	PxRigidBody* PRigidBody = FPhysicsInterface_PhysX::GetPxRigidBody_AssumesLocked(BodyInstance->GetPhysicsActorHandle());

	for (int32 i = 0; i < PhysTarget.Torques.Num(); ++i)
	{
		const FTorqueTarget& TorqueTarget = PhysTarget.Torques[i];
		PRigidBody->addTorque(U2PVector(TorqueTarget.Torque), TorqueTarget.bAccelChange ? PxForceMode::eACCELERATION : PxForceMode::eFORCE, true);
	}
#endif
#endif
}

/** Applies radial forces - Assumes caller has obtained writer lock */
void FPhysSubstepTask::ApplyRadialForces_AssumesLocked(const FPhysTarget& PhysTarget, FBodyInstance* BodyInstance)
{
#if WITH_PHYSX
#if WITH_CHAOS
    check(false);
#else
	/** Apply Torques */
	PxRigidBody* PRigidBody = FPhysicsInterface_PhysX::GetPxRigidBody_AssumesLocked(BodyInstance->GetPhysicsActorHandle());

	for (int32 i = 0; i < PhysTarget.RadialForces.Num(); ++i)
	{
		const FRadialForceTarget& RadialForceTArget= PhysTarget.RadialForces[i];
		AddRadialForceToPxRigidBody_AssumesLocked(*PRigidBody, RadialForceTArget.Origin, RadialForceTArget.Radius, RadialForceTArget.Strength, RadialForceTArget.Falloff, RadialForceTArget.bAccelChange);
	}
#endif
#endif
}


/** Interpolates kinematic actor transform - Assumes caller has obtained writer lock */
void FPhysSubstepTask::InterpolateKinematicActor_AssumesLocked(const FPhysTarget& PhysTarget, FBodyInstance* BodyInstance, float InAlpha)
{
#if WITH_PHYSX
#if WITH_CHAOS
    check(false);
#else
	PxRigidDynamic * PRigidDynamic = FPhysicsInterface_PhysX::GetPxRigidDynamic_AssumesLocked(BodyInstance->GetPhysicsActorHandle());
	InAlpha = FMath::Clamp(InAlpha, 0.f, 1.f);

	/** Interpolate kinematic actors */
	if (PhysTarget.bKinematicTarget)
	{
		//It's possible that the actor is no longer kinematic and is now simulating. In that case do nothing
		if (!BodyInstance->IsNonKinematic())
		{
			const FKinematicTarget& KinematicTarget = PhysTarget.KinematicTarget;
			const FTransform& TargetTM = KinematicTarget.TargetTM;
			const FTransform& StartTM = KinematicTarget.OriginalTM;
			FTransform InterTM = FTransform::Identity;

			InterTM.SetLocation(FMath::Lerp(StartTM.GetLocation(), TargetTM.GetLocation(), InAlpha));
			InterTM.SetRotation(FMath::Lerp(StartTM.GetRotation(), TargetTM.GetRotation(), InAlpha));

			PxTransform PNewPose = U2PTransform(InterTM);
			if (!ensureMsgf(PNewPose.isValid(), TEXT("Found an Invalid Pose for %s. Using previous pose."), *BodyInstance->GetBodyDebugName()))
			{
				PRigidDynamic->getKinematicTarget(PNewPose);
				if (!ensureMsgf(PNewPose.isValid(), TEXT("Previous pose is invalid. Using identity.")))
				{
					PNewPose = U2PTransform(FTransform::Identity);
				}
			}
			PRigidDynamic->setKinematicTarget(PNewPose);
		}
	}
#endif
#endif
}

void FPhysSubstepTask::SubstepInterpolation(float InAlpha, float DeltaTime)
{

}

float FPhysSubstepTask::UpdateTime(float UseDelta)
{
	float FrameRate = 1.f;
	uint32 MaxSubSteps = 1;

	UPhysicsSettings * PhysSetting = UPhysicsSettings::Get();
	FrameRate = PhysSetting->MaxSubstepDeltaTime;
	MaxSubSteps = PhysSetting->MaxSubsteps;
	
	float FrameRateInv = 1.f / FrameRate;

	//Figure out how big dt to make for desired framerate
	DeltaSeconds = FMath::Min(UseDelta, MaxSubSteps * FrameRate);
	NumSubsteps = FMath::CeilToInt(DeltaSeconds * FrameRateInv);
	NumSubsteps = FMath::Max(NumSubsteps > MaxSubSteps ? MaxSubSteps : NumSubsteps, (uint32) 1);
	SubTime = DeltaSeconds / NumSubsteps;

	return SubTime;
}

DECLARE_CYCLE_STAT(TEXT("Phys SubstepStart"), STAT_SubstepSimulationStart, STATGROUP_Physics);
DECLARE_CYCLE_STAT(TEXT("Phys SubstepEnd"), STAT_SubstepSimulationEnd, STATGROUP_Physics);

void FPhysSubstepTask::SubstepSimulationStart()
{
	SCOPE_CYCLE_COUNTER(STAT_TotalPhysicsTime);
	SCOPE_CYCLE_COUNTER(STAT_SubstepSimulationStart);
}

void FPhysSubstepTask::SubstepSimulationEnd(ENamedThreads::Type CurrentThread, const FGraphEventRef& MyCompletionGraphEvent)
{

}
