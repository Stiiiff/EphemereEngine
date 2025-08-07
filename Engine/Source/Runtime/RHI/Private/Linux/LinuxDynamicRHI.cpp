// Copyright Epic Games, Inc. All Rights Reserved.

#include "CoreMinimal.h"
#include "Misc/MessageDialog.h"
#include "RHI.h"
#include "Modules/ModuleManager.h"
#include "Misc/ConfigCacheIni.h"
#include "HAL/PlatformApplicationMisc.h"

FDynamicRHI* PlatformCreateDynamicRHI()
{
	ERHIFeatureLevel::Type RequestedFeatureLevel = ERHIFeatureLevel::SM5;
	FDynamicRHI* DynamicRHI = nullptr;

	const bool bForceVulkan = FParse::Param(FCommandLine::Get(), TEXT("vulkan"));

	bool bVulkanFailed = false;

	IDynamicRHIModule* DynamicRHIModule = nullptr;

	TArray<FString> TargetedShaderFormats;
	GConfig->GetArray(TEXT("/Script/LinuxTargetPlatform.LinuxTargetSettings"), TEXT("TargetedRHIs"), TargetedShaderFormats, GEngineIni);

	// First come first serve
	for (int32 SfIdx = 0; SfIdx < TargetedShaderFormats.Num(); ++SfIdx)
	{
		// If we havent failed to create a VulkanRHI try to again with a different TargetedRHI
		if (!bVulkanFailed && TargetedShaderFormats[SfIdx].StartsWith(TEXT("SF_VULKAN_")))
		{
			DynamicRHIModule = &FModuleManager::LoadModuleChecked<IDynamicRHIModule>(TEXT("VulkanRHI"));
			if (!DynamicRHIModule->IsSupported())
			{
				DynamicRHIModule = nullptr;
				bVulkanFailed = true;
			}
			else
			{
				FApp::SetGraphicsRHI(TEXT("Vulkan"));
				FPlatformApplicationMisc::UsingVulkan();

				FName ShaderFormatName(*TargetedShaderFormats[SfIdx]);
				EShaderPlatform TargetedPlatform = ShaderFormatToLegacyShaderPlatform(ShaderFormatName);
				RequestedFeatureLevel = GetMaxSupportedFeatureLevel(TargetedPlatform);
				break;
			}
		}
		else if (!bForceVulkan && TargetedShaderFormats[SfIdx].StartsWith(TEXT("GLSL_")))
		{
			DynamicRHIModule = nullptr;
		}
	}

	// Create the dynamic RHI.
	if (DynamicRHIModule)
	{
		DynamicRHI = DynamicRHIModule->CreateRHI(RequestedFeatureLevel);
	}
	else
	{
		if (bForceVulkan)
		{
			if (bVulkanFailed)
			{
				FMessageDialog::Open(EAppMsgType::Ok, NSLOCTEXT("LinuxDynamicRHI", "RequiredVulkan", "Vulkan Driver is required to run the engine."));
			}
			else
			{
				FMessageDialog::Open(EAppMsgType::Ok, NSLOCTEXT("LinuxDynamicRHI", "NoVulkanTargetedRHI", "Trying to force Vulkan RHI but the project does not have it in TargetedRHIs list."));
			}

			FPlatformMisc::RequestExitWithStatus(true, 1);
			// unreachable
			return nullptr;
		}
		else
		{
			if (bVulkanFailed)
			{
				FMessageDialog::Open(EAppMsgType::Ok, NSLOCTEXT("LinuxDynamicRHI", "NoVulkanDriver", "Failed to load Vulkan Driver which is required to run the engine."));
			}
			else
			{
				FMessageDialog::Open(EAppMsgType::Ok, NSLOCTEXT("LinuxDynamicRHI", "NoTargetedRHI", "The project does not target Vulkan, check project settings or pass -nullrhi."));
			}

			FPlatformMisc::RequestExitWithStatus(true, 1);
			// unreachable
			return nullptr;
		}
	}

	return DynamicRHI;
}
