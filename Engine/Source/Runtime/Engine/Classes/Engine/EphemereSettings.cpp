// Copyright DarkbloomVisuals. All Right Reserved.

#include "Engine/EphemereSettings.h"
#include "Curves/CurveLinearColorAtlas.h"
#include "Subsystems/EphemereSettingsSubsystem.h"

UEphemereSettings::UEphemereSettings(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{

}

void UEphemereSettings::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	// If a property change
	if (PropertyChangedEvent.Property != nullptr)
	{
		// If the changed property is ShadingCurveAtlas
		if (PropertyChangedEvent.Property->GetFName() == GET_MEMBER_NAME_CHECKED(UEphemereSettings, ShadingCurveAtlas))
		{
			// Try to load the shading atlas and check if it's not null
			UCurveLinearColorAtlas* ObjectAtlas = ShadingCurveAtlas.LoadSynchronous();
			if (ObjectAtlas)
			{
				UEphemereSettingsSubsystem* EphemereSettingsSubsystem = GEngine->GetEngineSubsystem<UEphemereSettingsSubsystem>();
				// The logic after loading the new atlas
				EphemereSettingsSubsystem->UpdateShadingCurveAtlas(ObjectAtlas);
			}

		}
	}
}
