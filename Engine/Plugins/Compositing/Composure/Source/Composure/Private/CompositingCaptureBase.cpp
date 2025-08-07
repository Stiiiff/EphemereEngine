// Copyright Epic Games, Inc. All Rights Reserved.

#include "CompositingCaptureBase.h"

#include "CineCameraComponent.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Engine/Engine.h"

ACompositingCaptureBase::ACompositingCaptureBase()
{
	// Create the SceneCapture component and assign its parent to be the RootComponent (the ComposurePostProcessingPassProxy)
	SceneCaptureComponent2D = CreateDefaultSubobject<USceneCaptureComponent2D>(TEXT("SceneCaptureComponent"));
	SceneCaptureComponent2D->SetupAttachment(RootComponent);

	// The SceneCaptureComponent2D default constructor disables TAA, but CG Compositing Elements enable it by default
	SceneCaptureComponent2D->ShowFlags.TemporalAA = true;
}

void ACompositingCaptureBase::UpdateDistortion()
{
	// Get the TargetCameraActor associated with this CG Layer
	ACameraActor* TargetCamera = FindTargetCamera();
	if (TargetCamera == nullptr)
	{
		return;
	}
	
 	if (UCineCameraComponent* const CineCameraComponent = Cast<UCineCameraComponent>(TargetCamera->GetCameraComponent()))
	{
		{
			OverscanFactor = 1.0f;

			/*if (SceneCaptureComponent2D)
			{
				SceneCaptureComponent2D->RemoveBlendable(LastDistortionMID);
			}*/
			LastDistortionMID = nullptr;
		}
	}
}

#if WITH_EDITOR
void ACompositingCaptureBase::PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)
{
	const FName PropertyName = (PropertyChangedEvent.Property != NULL) ? PropertyChangedEvent.Property->GetFName() : NAME_None;

	if ((PropertyName == GET_MEMBER_NAME_CHECKED(ACompositingCaptureBase, TargetCameraActor)))
	{
		// If there is no target camera, remove the last distortion post-process MID from the scene capture
		if (TargetCameraActor == nullptr)
		{
			/*if (SceneCaptureComponent2D)
			{
				SceneCaptureComponent2D->RemoveBlendable(LastDistortionMID);
			}*/

			LastDistortionMID = nullptr;

			return;
		}
	}

	Super::PostEditChangeProperty(PropertyChangedEvent);
}
#endif

void ACompositingCaptureBase::SetApplyDistortion(bool bInApplyDistortion)
{
	bApplyDistortion = bInApplyDistortion;
	UpdateDistortion();
}
