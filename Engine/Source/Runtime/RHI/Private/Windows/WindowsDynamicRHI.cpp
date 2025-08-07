// Copyright Epic Games, Inc. All Rights Reserved.

#include "RHI.h"
#include "Modules/ModuleManager.h"
#include "Misc/CommandLine.h"
#include "Misc/ConfigCacheIni.h"
#include "Misc/MessageDialog.h"

#if WINDOWS_USE_FEATURE_DYNAMIC_RHI

#include "Windows/WindowsPlatformApplicationMisc.h"

static const TCHAR* GLoadedRHIModuleName;

static IDynamicRHIModule* LoadDynamicRHIModule(ERHIFeatureLevel::Type& DesiredFeatureLevel, const TCHAR*& LoadedRHIModuleName)
{
	bool bUseGPUCrashDebugging = false;
	if (!GIsEditor && GConfig->GetBool(TEXT("D3DRHIPreference"), TEXT("bUseGPUCrashDebugging"), bUseGPUCrashDebugging, GGameUserSettingsIni))
	{
		auto GPUCrashDebuggingCVar = IConsoleManager::Get().FindTConsoleVariableDataInt(TEXT("r.GPUCrashDebugging"));
		*GPUCrashDebuggingCVar = bUseGPUCrashDebugging;
	}

	bool bForceSM5 = FParse::Param(FCommandLine::Get(), TEXT("sm5"));
	bool bForceVulkan = FParse::Param(FCommandLine::Get(), TEXT("vulkan"));
	DesiredFeatureLevel = ERHIFeatureLevel::Num;
	
	if(!(bForceVulkan))
	{
		//Default graphics RHI is only used if no command line option is specified
		FConfigFile EngineSettings;
		FString PlatformNameString = FPlatformProperties::PlatformName();
		const TCHAR* PlatformName = *PlatformNameString;
		FConfigCacheIni::LoadLocalIniFile(EngineSettings, TEXT("Engine"), true, PlatformName);
		FString DefaultGraphicsRHI;
		if(EngineSettings.GetString(TEXT("/Script/WindowsTargetPlatform.WindowsTargetSettings"), TEXT("DefaultGraphicsRHI"), DefaultGraphicsRHI))
		{
			FString NAME_VULKAN(TEXT("DefaultGraphicsRHI_Vulkan"));
			if (DefaultGraphicsRHI == NAME_VULKAN)
			{
				bForceVulkan = true;
			}
		}
	}

	int32 Sum = ((0) + (0) + (0) + (bForceVulkan ? 1 : 0));

	if (Sum > 1)
	{
		UE_LOG(LogRHI, Fatal, TEXT("-vulkan is the only mutually exclusive option, but more than one was specified on the command-line."));
	}
	else if (Sum == 0)
	{
		// Check the list of targeted shader platforms and decide an RHI based off them
		TArray<FString> TargetedShaderFormats;
		GConfig->GetArray(TEXT("/Script/WindowsTargetPlatform.WindowsTargetSettings"), TEXT("TargetedRHIs"), TargetedShaderFormats, GEngineIni);
		if (TargetedShaderFormats.Num() > 0)
		{
			// Pick the first one
			FName ShaderFormatName(*TargetedShaderFormats[0]);
			EShaderPlatform TargetedPlatform = ShaderFormatToLegacyShaderPlatform(ShaderFormatName);
			bForceVulkan = IsVulkanPlatform(TargetedPlatform);
			DesiredFeatureLevel = GetMaxSupportedFeatureLevel(TargetedPlatform);
		}
	}
	else
	{
		DesiredFeatureLevel = ERHIFeatureLevel::SM5;
	}

	// Load the dynamic RHI module.
	IDynamicRHIModule* DynamicRHIModule = NULL;

#if defined(SWITCHRHI)
	const bool bForceSwitch = FParse::Param(FCommandLine::Get(), TEXT("switch"));
	// Load the dynamic RHI module.
	if (bForceSwitch)
	{
#define A(x) #x
#define B(x) A(x)
#define SWITCH_RHI_STR B(SWITCHRHI)
		FApp::SetGraphicsRHI(TEXT("Switch"));
		const TCHAR* SwitchRHIModuleName = TEXT(SWITCH_RHI_STR);
		DynamicRHIModule = &FModuleManager::LoadModuleChecked<IDynamicRHIModule>(SwitchRHIModuleName);
		if (!DynamicRHIModule->IsSupported())
		{
			FMessageDialog::Open(EAppMsgType::Ok, NSLOCTEXT("SwitchDynamicRHI", "UnsupportedRHI", "The chosen RHI is not supported"));
			FPlatformMisc::RequestExit(1);
			DynamicRHIModule = NULL;
		}
		LoadedRHIModuleName = SwitchRHIModuleName;
	}
	else
#endif

	if (bForceVulkan)
	{
		FApp::SetGraphicsRHI(TEXT("Vulkan"));
		const TCHAR* VulkanRHIModuleName = TEXT("VulkanRHI");
		DynamicRHIModule = &FModuleManager::LoadModuleChecked<IDynamicRHIModule>(VulkanRHIModuleName);
		if (!DynamicRHIModule->IsSupported())
		{
			FMessageDialog::Open(EAppMsgType::Ok, NSLOCTEXT("WindowsDynamicRHI", "RequiredVulkan", "Vulkan Driver is required to run the engine."));
			FPlatformMisc::RequestExit(1);
			DynamicRHIModule = NULL;
		}
		LoadedRHIModuleName = VulkanRHIModuleName;
	}
	
	return DynamicRHIModule;
}

FDynamicRHI* PlatformCreateDynamicRHI()
{
	FDynamicRHI* DynamicRHI = nullptr;

#if UE_BUILD_DEBUG || UE_BUILD_DEVELOPMENT
	if (!FPlatformMisc::IsDebuggerPresent())
	{
		if (FParse::Param(FCommandLine::Get(), TEXT("AttachDebugger")))
		{
			// Wait to attach debugger
			do
			{
				FPlatformProcess::Sleep(0);
			}
			while (!FPlatformMisc::IsDebuggerPresent());
		}
	}
#endif

	ERHIFeatureLevel::Type RequestedFeatureLevel;
	const TCHAR* LoadedRHIModuleName;
	IDynamicRHIModule* DynamicRHIModule = LoadDynamicRHIModule(RequestedFeatureLevel, LoadedRHIModuleName);

	if (DynamicRHIModule)
	{
		// Create the dynamic RHI.
		DynamicRHI = DynamicRHIModule->CreateRHI(RequestedFeatureLevel);
		GLoadedRHIModuleName = LoadedRHIModuleName;
	}

	return DynamicRHI;
}

const TCHAR* GetSelectedDynamicRHIModuleName(bool bCleanup)
{
	check(FApp::CanEverRender());
	if (GDynamicRHI)
	{
		check(!!GLoadedRHIModuleName);
		return GLoadedRHIModuleName;
	}
	else
	{
		ERHIFeatureLevel::Type DesiredFeatureLevel;
		const TCHAR* RHIModuleName;
		IDynamicRHIModule* DynamicRHIModule = LoadDynamicRHIModule(DesiredFeatureLevel, RHIModuleName);
		check(DynamicRHIModule);
		check(RHIModuleName);
		if (bCleanup)
		{
			FModuleManager::Get().UnloadModule(RHIModuleName);
		}
		return RHIModuleName;
	}
}

#endif //WINDOWS_USE_FEATURE_DYNAMIC_RHI
