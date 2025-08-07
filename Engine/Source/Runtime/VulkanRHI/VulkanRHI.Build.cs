// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

[SupportedPlatforms("AllDesktop")]
public class VulkanRHI : ModuleRules
{
	public VulkanRHI(ReadOnlyTargetRules Target) : base(Target)
	{
		bLegalToDistributeObjectCode = true;
		bBuildLocallyWithSNDBS = false; // VulkanPlatform.h

		if (Target.Platform.IsInGroup(UnrealPlatformGroup.Windows))
		{
			AddEngineThirdPartyPrivateStaticDependencies(Target, "AMD_AGS");
		}

		if (Target.Platform.IsInGroup(UnrealPlatformGroup.Windows))
		{
			PublicDefinitions.Add("VK_USE_PLATFORM_WIN32_KHR=1");
			PublicDefinitions.Add("VK_USE_PLATFORM_WIN32_KHX=1");
		}

		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"Core", 
				"CoreUObject",
				"ApplicationCore",
				"Engine", 
				"RHI",
				"RenderCore", 
				"PreLoadScreen",
				"BuildSettings",
				"TraceLog",
			}
		);

		if (Target.Platform.IsInGroup(UnrealPlatformGroup.Windows) || Target.IsInPlatformGroup(UnrealPlatformGroup.Linux))
		{
			AddEngineThirdPartyPrivateStaticDependencies(Target, "NVAftermath");
		}

		if (Target.Platform.IsInGroup(UnrealPlatformGroup.Windows) || Target.IsInPlatformGroup(UnrealPlatformGroup.Unix))
		{
            AddEngineThirdPartyPrivateStaticDependencies(Target, "Vulkan");
			PublicIncludePathModuleNames.Add("Vulkan");
        }

		if (Target.IsInPlatformGroup(UnrealPlatformGroup.Linux))
		{
			PrivateDependencyModuleNames.Add("ApplicationCore");
			AddEngineThirdPartyPrivateStaticDependencies(Target, "SDL2");
		}
		else
		{
			PrecompileForTargets = PrecompileTargetsType.None;
		}
	}
}
