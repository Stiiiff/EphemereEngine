// Copyright DarkbloomVisuals. All Right Reserved.

#include "Subsystems/ShadingSubsystem.h"
#include "Engine/ShadingSettings.h"
#include "Curves/CurveLinearColorAtlas.h"

void UShadingSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// We get our default shading settings class
	const UShadingSettings* ShadingSettings = GetDefault<UShadingSettings>();

	// We load our shading curve atlas
	ShadingCurveAtlas = ShadingSettings->ShadingCurveAtlas.LoadSynchronous();

	// If the shading atlas is nullptr, display an error
	if (!ShadingCurveAtlas)
	{
		UE_LOG(LogInit, Error, TEXT("Can't load shading atlas, please check selected asset in project settings"));
	}
}

void UShadingSubsystem::Deinitialize()
{
	// Remove the reference count
	ShadingCurveAtlas = nullptr;
}

void UShadingSubsystem::UpdateShadingCurveAtlas(UCurveLinearColorAtlas* InAtlas)
{
	ShadingCurveAtlas = InAtlas;
}

UCurveLinearColorAtlas* UShadingSubsystem::GetShadingCurveAtlas() const
{
	return ShadingCurveAtlas;
}