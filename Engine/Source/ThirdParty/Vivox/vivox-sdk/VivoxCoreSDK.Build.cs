// Copyright Epic Games, Inc. All Rights Reserved.

using System.IO;

namespace UnrealBuildTool.Rules
{
	public class VivoxCoreSDK : ModuleRules
	{
		public VivoxCoreSDK(ReadOnlyTargetRules Target) : base(Target)
		{
			Type = ModuleType.External;

			string VivoxSDKPath = ModuleDirectory;
			string PlatformSubdir = Target.Platform.ToString();
			string VivoxLibPath = Path.Combine(VivoxSDKPath, "Lib", PlatformSubdir) + "/";
			string VivoxIncludePath = Path.Combine(VivoxSDKPath, "Include");
			PublicIncludePaths.Add(VivoxIncludePath);

			if (Target.Platform == UnrealTargetPlatform.Win64)
			{
				PublicAdditionalLibraries.Add(VivoxLibPath + "vivoxsdk_x64.lib");
				PublicDelayLoadDLLs.Add("ortp_x64.dll");
				PublicDelayLoadDLLs.Add("vivoxsdk_x64.dll");
				RuntimeDependencies.Add(Path.Combine("$(TargetOutputDir)", "ortp_x64.dll"), Path.Combine(VivoxLibPath, "ortp_x64.dll"));
				RuntimeDependencies.Add(Path.Combine("$(TargetOutputDir)", "vivoxsdk_x64.dll"), Path.Combine(VivoxLibPath, "vivoxsdk_x64.dll"));
			}
		}
	}
}
