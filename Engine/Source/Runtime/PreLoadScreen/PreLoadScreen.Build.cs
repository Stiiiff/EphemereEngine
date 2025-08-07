// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class PreLoadScreen : ModuleRules
{
	public PreLoadScreen(ReadOnlyTargetRules Target) : base(Target)
	{
		PublicIncludePaths.Add("Runtime/PreLoadScreen/Public");
		PrivateIncludePaths.Add("Runtime/PreLoadScreen/Private");

		PublicDependencyModuleNames.AddRange(
			new string[] {
					"Engine",
					"ApplicationCore",
					"Analytics",
					"AnalyticsET",
				}
		);

		PrivateDependencyModuleNames.AddRange(
			new string[] {
					"Core",
					"InputCore",
					"RenderCore",
					"CoreUObject",
					"RHI",
					"Slate",
					"SlateCore",
					"BuildPatchServices",
					"Projects",
			}
		);
	}
}
