// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.IO;

public class libOpus : ModuleRules
{
	protected virtual string OpusVersion { get { return "opus-1.1"; } }
	protected virtual string IncRootDirectory { get { return Target.UEThirdPartySourceDirectory; } }
	protected virtual string LibRootDirectory { get { return Target.UEThirdPartySourceDirectory; } }

	protected virtual string OpusIncPath { get { return Path.Combine(IncRootDirectory, "libOpus", OpusVersion, "include"); } }
	protected virtual string OpusLibPath { get { return Path.Combine(LibRootDirectory, "libOpus", OpusVersion); } }

	public libOpus(ReadOnlyTargetRules Target) : base(Target)
	{
		Type = ModuleType.External;

		string LibraryPath = OpusLibPath + "/";
		bool bIsNewOpus = Target.Platform == UnrealTargetPlatform.Win64 || Target.Architecture == "x86_64-unknown-linux-gnu";
		string OpusLibraryPath = Path.Combine(LibRootDirectory, "libOpus", "opus-1.3.1-12");

		if ((Target.Platform == UnrealTargetPlatform.Win64))
		{
			LibraryPath += "Windows/VS2012/";
			if (Target.Platform == UnrealTargetPlatform.Win64)
			{
				LibraryPath += "x64/";
			}
			else
			{
				LibraryPath += "win32/";
			}

			LibraryPath += "Release/";

			//PublicAdditionalLibraries.Add(LibraryPath + "silk_common.lib");
			//PublicAdditionalLibraries.Add(LibraryPath + "silk_float.lib");
			//PublicAdditionalLibraries.Add(LibraryPath + "celt.lib");
			//PublicAdditionalLibraries.Add(LibraryPath + "opus.lib");
			PublicAdditionalLibraries.Add(LibraryPath + "speex_resampler.lib");

			string ConfigPath = "";
			if (Target.Configuration == UnrealTargetConfiguration.Debug && Target.bDebugBuildsActuallyUseDebugCRT)
			{
				ConfigPath = "Debug";
			}
			else
			{
				ConfigPath = "Release";
			}

			string OpusBinaryPath = Path.Combine(OpusLibraryPath, "bin", Target.Platform.ToString(), ConfigPath);
			PublicAdditionalLibraries.Add(Path.Combine(OpusBinaryPath, "opus.lib"));
			PublicAdditionalLibraries.Add(Path.Combine(OpusBinaryPath, "opus_sse41.lib"));
		}
		else if (Target.IsInPlatformGroup(UnrealPlatformGroup.Unix))
		{
			// Only x86_64 was rebuilt with newer Opus
			if (Target.Architecture.StartsWith("x86_64"))
			{
				string Config = Target.Configuration == UnrealTargetConfiguration.Debug ? "Debug" : "Release";

				PublicAdditionalLibraries.Add(Path.Combine(OpusLibraryPath, "bin", "Linux", Target.Architecture, Config, "libopus.a"));
				PublicAdditionalLibraries.Add(Path.Combine(OpusLibraryPath, "bin", "Linux", Target.Architecture, Config, "libopus_sse41.a"));

				PublicAdditionalLibraries.Add(Path.Combine(OpusLibPath, "Linux", Target.Architecture, "libresampler_fPIC.a"));
			}
			else
			{
				PublicAdditionalLibraries.Add(Path.Combine(OpusLibPath, "Linux", Target.Architecture, "libopus.a"));
			}
		}

		PublicIncludePaths.Add(bIsNewOpus ? Path.Combine(OpusLibraryPath, "include") : OpusIncPath);
	}
}
