// Copyright DarkbloomVisuals. All Right Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "Engine/DeveloperSettings.h"

#include "EphemereSettings.generated.h"

class UEphemereSettings;

/**
* Additionnal settings to control the rendering of the engine depending on the game.
*/

UCLASS(config = Engine, defaultconfig, meta = (DisplayName = "Ephemere Settings"))
class ENGINE_API UEphemereSettings : public UDeveloperSettings
{
	GENERATED_UCLASS_BODY()

public:
	UPROPERTY(EditAnywhere, config, Category = Shading, meta = (DisplayName = "Shading Curve Atlas",
		ToolTip = "Linear color curve altas to pick the curve used for shading calculations.",
		ConfigRestartRequired = false))
	TSoftObjectPtr<UCurveLinearColorAtlas> ShadingCurveAtlas;

	UPROPERTY(EditAnywhere, config, Category = Shading, meta = (DisplayName = "Curve Atlas Size",
		ToolTip = "Control the max number of shading curve.",
		ConfigRestartRequired = true))
	int32 MaxShadingCurves = 64;

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
};