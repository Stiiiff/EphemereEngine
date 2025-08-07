// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class ApplicationCore : ModuleRules
{
	public ApplicationCore(ReadOnlyTargetRules Target) : base(Target)
	{
		PublicDependencyModuleNames.AddRange(
			new string[] {
				"Core"
			}
		);

		PublicIncludePathModuleNames.AddRange(
			new string[] {
				"RHI"
			}
		);

		PrivateIncludePathModuleNames.AddRange(
			new string[] {
				"InputDevice",
				"Analytics",
				"SynthBenchmark"
			}
		);

		if (Target.IsInPlatformGroup(UnrealPlatformGroup.Windows))
		{
			if (Target.bCompileWithAccessibilitySupport && !Target.bIsBuildingConsoleApplication)
			{
				PublicSystemLibraries.Add("uiautomationcore.lib");
			}

			// Uses DXGI to query GPU hardware prior to RHI startup
			PublicSystemLibraries.Add("DXGI.lib");
		}
		else if (Target.IsInPlatformGroup(UnrealPlatformGroup.Linux))
		{
			AddEngineThirdPartyPrivateStaticDependencies(Target,
				"SDL2"
			);

			// We need FreeType2 and GL for the Splash, but only in the Editor
			if (Target.Type == TargetType.Editor)
			{
				AddEngineThirdPartyPrivateStaticDependencies(Target, "FreeType2");
				PrivateIncludePathModuleNames.Add("ImageWrapper");
			}
		}

		if (!Target.bCompileAgainstApplicationCore)
		{
			throw new System.Exception("ApplicationCore cannot be used when Target.bCompileAgainstApplicationCore = false.");
		}
	}
}
