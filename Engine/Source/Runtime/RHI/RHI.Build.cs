// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System;

public class RHI : ModuleRules
{
	public RHI(ReadOnlyTargetRules Target) : base(Target)
	{
		PrivateDependencyModuleNames.Add("Core");
		PrivateDependencyModuleNames.Add("TraceLog");
		PrivateDependencyModuleNames.Add("ApplicationCore");
		PrivateDependencyModuleNames.Add("GeForceNOWWrapper");

		if (Target.bCompileAgainstEngine)
		{
			DynamicallyLoadedModuleNames.Add("NullDrv");

			if (Target.Type != TargetRules.TargetType.Server)   // Dedicated servers should skip loading everything but NullDrv
			{
				if ((Target.Platform == UnrealTargetPlatform.Win64) ||
					(Target.IsInPlatformGroup(UnrealPlatformGroup.Unix) && (Target.Architecture.StartsWith("x86_64") || Target.Architecture.StartsWith("aarch64"))))	// temporary, not all archs can support Vulkan atm
				{
					DynamicallyLoadedModuleNames.Add("VulkanRHI");
				}
			}
		}

		if (Target.Configuration != UnrealTargetConfiguration.Shipping)
		{
			PrivateIncludePathModuleNames.AddRange(new string[] { "TaskGraph" });
		}

		PrivateIncludePaths.Add("Runtime/RHI/Private");
    }
}
