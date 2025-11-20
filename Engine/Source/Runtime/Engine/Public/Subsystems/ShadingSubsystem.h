// Copyright DarkbloomVisuals. All Right Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/EngineSubsystem.h"

#include "ShadingSubsystem.generated.h"

class UShadingSubsystem

/**
 * UCelshadingSubsystem
 * Celshading subsystem to load and manage curve atlas.
 */

UCLASS()
class ENGINE_API UShadingSubsystem : public UEngineSubsystem
{
	GENERATED_BODY()

public:
	// Initalize the subsystem, we load the curve atlas here
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	// Triggered at the end of the subsystem lifetime, for cleanup
	virtual void Deinitialize() override;

	// Getter for the shading curve atlas
	TObjectPtr<UCurveLinearColorAtlas> GetShadingCurveAtlas() const;

	// Update the reference of the shading curve atlas
	void UpdateShadingCurveAtlas(UCurveLinearColorAtlas* InAtlas);

private:
	// The reference to the shading linear color curve atlas
	UPROPERTY()
	TObjectPtr<UCurveLinearColorAtlas> ShadingCurveAtlas;
};
