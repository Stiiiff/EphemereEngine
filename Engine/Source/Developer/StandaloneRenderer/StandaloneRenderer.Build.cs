// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class StandaloneRenderer : ModuleRules
{
	public StandaloneRenderer(ReadOnlyTargetRules Target) : base(Target)
	{
		PrivateIncludePaths.Add("Developer/StandaloneRenderer/Private");

		PrivateDependencyModuleNames.AddRange(
			new string[] {
				"Core",
				"CoreUObject",
				"ApplicationCore",
				"ImageWrapper",
				"InputCore",
				"SlateCore",
			}
			);

		AddEngineThirdPartyPrivateStaticDependencies(Target, "SDL2");

		RuntimeDependencies.Add("$(EngineDir)/Shaders/StandaloneRenderer/...", StagedFileType.UFS);
	}
}
