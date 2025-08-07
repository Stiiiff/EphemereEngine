// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.IO;

public class zlib : ModuleRules
{
	protected string CurrentZlibVersion;

	public zlib(ReadOnlyTargetRules Target) : base(Target)
	{
		Type = ModuleType.External;

		CurrentZlibVersion = "v1.2.8";

		string zlibPath = Target.UEThirdPartySourceDirectory + "zlib/" + CurrentZlibVersion;

		// On Windows x64, use the llvm compiled version which is quite a bit faster than the MSVC compiled version.
		if (Target.Platform == UnrealTargetPlatform.Win64 &&
		    Target.WindowsPlatform.Architecture == WindowsArchitecture.x64)
		{
			string LibDir  = System.String.Format("{0}/lib/Win64-llvm/{1}/", zlibPath, Target.Configuration != UnrealTargetConfiguration.Debug ? "Release" : "Debug");
			string LibName = System.String.Format("zlibstatic{0}.lib", Target.Configuration != UnrealTargetConfiguration.Debug ? "" : "d");
			PublicAdditionalLibraries.Add(LibDir + LibName);

			string PlatformSubpath = "Win64";
			PublicIncludePaths.Add(System.String.Format("{0}/include/{1}/VS{2}", zlibPath, PlatformSubpath, Target.WindowsPlatform.GetVisualStudioCompilerVersionName()));
		}
		else if (Target.Platform == UnrealTargetPlatform.Win64)
		{
			string PlatformSubpath = "Win64";
			PublicIncludePaths.Add(System.String.Format("{0}/include/{1}/VS{2}", zlibPath, PlatformSubpath, Target.WindowsPlatform.GetVisualStudioCompilerVersionName()));
			string LibDir;

			if (Target.WindowsPlatform.Architecture == WindowsArchitecture.ARM32 || Target.WindowsPlatform.Architecture == WindowsArchitecture.ARM64)
            {
                LibDir = System.String.Format("{0}/lib/{1}/VS{2}/{3}/", zlibPath, PlatformSubpath, Target.WindowsPlatform.GetVisualStudioCompilerVersionName(), Target.WindowsPlatform.GetArchitectureSubpath());
            }
            else
            {
                LibDir = System.String.Format("{0}/lib/{1}/VS{2}/{3}/", zlibPath, PlatformSubpath, Target.WindowsPlatform.GetVisualStudioCompilerVersionName(), Target.Configuration == UnrealTargetConfiguration.Debug ? "Debug" : "Release");
            }
            PublicAdditionalLibraries.Add(LibDir + "zlibstatic.lib");
        }
		else if (Target.IsInPlatformGroup(UnrealPlatformGroup.Unix))
		{
			string platform = "/Linux/" + Target.Architecture;
			PublicIncludePaths.Add(zlibPath + "/include" + platform);
			PublicAdditionalLibraries.Add(zlibPath + "/lib/" + platform + "/libz_fPIC.a");
		}
	}
}
