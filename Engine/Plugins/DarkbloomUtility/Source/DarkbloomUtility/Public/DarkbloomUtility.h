// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

class FDarkbloomUtilityModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void InjectGlobalDefines(FShaderCompilerEnvironment& OutEnv) const;
	virtual void ShutdownModule() override;
};
