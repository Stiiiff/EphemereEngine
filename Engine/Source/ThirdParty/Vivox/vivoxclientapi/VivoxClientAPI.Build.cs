// Copyright Epic Games, Inc. All Rights Reserved.
using UnrealBuildTool;
using System.IO;

public class VivoxClientAPI : ModuleRules
{
	public VivoxClientAPI(ReadOnlyTargetRules Target) : base(Target)
	{
		Type = ModuleType.External;

		PublicDependencyModuleNames.Add("VivoxCoreSDK");

		string VivoxClientAPIPath = ModuleDirectory;
		string PlatformSubdir = Target.Platform.ToString();

		bool bUseDebugBuild = (Target.Configuration == UnrealTargetConfiguration.Debug && Target.bDebugBuildsActuallyUseDebugCRT);
		string ConfigurationSubdir = bUseDebugBuild ? "Debug" : "Release";

		PublicIncludePaths.Add(Path.Combine(VivoxClientAPIPath, "vivoxclientapi", "include"));

		if (Target.Platform == UnrealTargetPlatform.Win64)
		{
			string LibDir = Path.Combine(VivoxClientAPIPath, PlatformSubdir, "VS" + Target.WindowsPlatform.GetVisualStudioCompilerVersionName(), ConfigurationSubdir);
			PublicAdditionalLibraries.Add(LibDir + "/vivoxclientapi.lib");
		}
	}
}
