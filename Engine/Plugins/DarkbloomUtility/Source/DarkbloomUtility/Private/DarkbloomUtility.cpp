// Copyright Epic Games, Inc. All Rights Reserved.

#include "DarkbloomUtility.h"
#include "Engine/EphemereSettings.h"
#include "ShaderCore.h"

#define LOCTEXT_NAMESPACE "FDarkbloomUtilityModule"

void FDarkbloomUtilityModule::StartupModule()
{
	FShaderCompilerEnvironment::ModifyCompilationEnvironment.AddRaw(
		this, &FDarkbloomUtilityModule::InjectGlobalDefines);
}

void FDarkbloomUtilityModule::InjectGlobalDefines(FShaderCompilerEnvironment& OutEnv) const
{
	const UEphemereSettings* Settings = GetDefault<UEphemereSettings>();
	int32 MaxShadingCurves = Settings->MaxShadingCurves;
	OutEnv.SetDefine(TEXT("MAX_SHADING_CURVES"), MaxShadingCurves);
}

void FDarkbloomUtilityModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FDarkbloomUtilityModule, DarkbloomUtility)