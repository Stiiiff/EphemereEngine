// Copyright DarkbloomVisuals. All Right Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/EngineSubsystem.h"

#include "EphemereSettingsSubsystem.generated.h"

class UEphemereSettingsSubsystem;

/**
 * UEphemereSettingsSubsystem
 * EphemereSettings subsystem to load and Ephemere Settings.
 */

UCLASS()
class ENGINE_API UEphemereSettingsSubsystem : public UEngineSubsystem
{
	GENERATED_BODY()

public:
	// Initialize the subsystem
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	// Triggered at the end of the subsystem lifetime, for cleanup
	virtual void Deinitialize() override;

	// Getter for the shading curve atlas
	UCurveLinearColorAtlas* GetShadingCurveAtlas() const;

	// Update the reference of the shading curve atlas
	void UpdateShadingCurveAtlas(UCurveLinearColorAtlas* InAtlas);

	// Getter for the max shading curves
	int32 GetMaxShadingCurves() const;

	// Update the max shading curves
	void UpdateMaxShadingCurves(int32 InMaxShadingCurves);

private:
	// The reference to the shading linear color curve atlas
	UPROPERTY()
	UCurveLinearColorAtlas* ShadingCurveAtlas;

	// The reference to the max shading curves
	UPROPERTY()
	int32 MaxShadingCurves;
};
