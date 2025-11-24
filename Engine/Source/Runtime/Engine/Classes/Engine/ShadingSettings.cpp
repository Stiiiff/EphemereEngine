// Copyright DarkbloomVisuals. All Right Reserved.

#include "Engine/ShadingSettings.h"
#include "Curves/CurveLinearColorAtlas.h"
#include "Subsystems/ShadingSubsystem.h"

UShadingSettings::UShadingSettings(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{

}

void UShadingSettings::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	// If a property change
	if (PropertyChangedEvent.Property != nullptr)
	{
		// If the changed property is ShadingCurveAtlas
		if (PropertyChangedEvent.Property->GetFName() == GET_MEMBER_NAME_CHECKED(UShadingSettings, ShadingCurveAtlas))
		{
			// Try to load the shading atlas and check if it's not null
			UCurveLinearColorAtlas* ObjectAtlas = ShadingCurveAtlas.LoadSynchronous();
			if (ObjectAtlas)
			{
				UShadingSubsystem* ShadingSubsystem = GEngine->GetEngineSubsystem<UShadingSubsystem>();
				// The logic after loading the new atlas
				ShadingSubsystem->UpdateShadingCurveAtlas(ObjectAtlas);
			}

		}
	}
}
