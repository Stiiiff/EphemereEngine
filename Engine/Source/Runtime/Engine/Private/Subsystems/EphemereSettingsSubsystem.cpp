// Copyright DarkbloomVisuals. All Right Reserved.

#include "Subsystems/EphemereSettingsSubsystem.h"
#include "Engine/EphemereSettings.h"
#include "Curves/CurveLinearColorAtlas.h"

void UEphemereSettingsSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// We get our default Ephemere settings class
	const UEphemereSettings* EphemereSettingsSettings = GetDefault<UEphemereSettings>();

	// We load our shading curve atlas
	ShadingCurveAtlas = EphemereSettingsSettings->ShadingCurveAtlas.LoadSynchronous();

	// If the shading atlas is nullptr, display an error
	if (!ShadingCurveAtlas)
	{
		UE_LOG(LogInit, Error, TEXT("Can't load shading atlas, please check selected asset in project settings"));
	}

	// We get the max shading curves
	MaxShadingCurves = EphemereSettingsSettings->MaxShadingCurves;
}

void UEphemereSettingsSubsystem::Deinitialize()
{
	// Remove the reference count
	ShadingCurveAtlas = nullptr;
}

void UEphemereSettingsSubsystem::UpdateShadingCurveAtlas(UCurveLinearColorAtlas* InAtlas)
{
	ShadingCurveAtlas = InAtlas;
}

void UEphemereSettingsSubsystem::UpdateMaxShadingCurves(int32 InMaxShadingCurves)
{
	MaxShadingCurves = InMaxShadingCurves;
}

UCurveLinearColorAtlas* UEphemereSettingsSubsystem::GetShadingCurveAtlas() const
{
	return ShadingCurveAtlas;
}

int32 UEphemereSettingsSubsystem::GetMaxShadingCurves() const
{
	return MaxShadingCurves;
}