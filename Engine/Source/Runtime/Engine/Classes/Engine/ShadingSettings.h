// Copyright DarkbloomVisuals. All Right Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "Engine/DeveloperSettings.h"

#include "ShadingSettings.generated.h"

class UShadingSettings;

/**
* Settings for shading properties.
*/

UCLASS(config = Engine, defaultconfig, meta = (DisplayName = "Shading Settings"))
class ENGINE_API UShadingSettings : public UDeveloperSettings
{
	GENERATED_UCLASS_BODY()

public:
	UPROPERTY(EditAnywhere, config, Category = General, meta = (DisplayName = "Shading Curve Atlas",
		ToolTip = "Linear color curve altas to pick the curve used for shading calculations.",
		ConfigRestartRequired = false))
	TSoftObjectPtr<UCurveLinearColorAtlas> ShadingCurveAtlas;

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
};