// Copyright Epic Games, Inc. All Rights Reserved.

using System.IO;
using System.Collections.Generic;
using System.Security.Cryptography;
using UnrealBuildTool;

public class OodleDataCompressionFormat : ModuleRules
{
	protected virtual string OodleVersion { get { return "2.9.0"; } }

	// Platform Extensions need to override these
	protected virtual string LibRootDirectory { get { return ModuleDirectory; } }
	protected virtual string ReleaseLibraryName { get { return null; } }
	protected virtual string DebugLibraryName { get { return null; } }

	protected virtual string SdkBaseDirectory { get { return Path.Combine(LibRootDirectory, "..", "Sdks", OodleVersion); } }
	protected virtual string LibDirectory { get { return Path.Combine(SdkBaseDirectory, "lib"); } }

	protected virtual string IncludeDirectory { get { return Path.Combine(ModuleDirectory, "..", "Sdks", OodleVersion, "include"); } }


	public OodleDataCompressionFormat(ReadOnlyTargetRules Target) : base(Target)
	{
		ShortName = "OodleDataCompressionFormat";

        PublicIncludePaths.Add(IncludeDirectory);

        PrivatePCHHeaderFile = "Private/OodleDataCompressionFormatPCH.h";

		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"CoreUObject",
				"Analytics"
			}
		);

        if (Target.bBuildEditor == true)
        {
            PrivateDependencyModuleNames.AddRange(new string[] { "UnrealEd" });
        }

		string ReleaseLib = null;
		string DebugLib = null;
		string PlatformDir = Target.Platform.ToString();

		// turn on bAllowDebugLibrary if you need to debug a problem with Oodle
		bool bAllowDebugLibrary = false;
		bool bUseDebugLibrary = bAllowDebugLibrary && Target.Configuration == UnrealTargetConfiguration.Debug && Target.bDebugBuildsActuallyUseDebugCRT;

		bool bSkipLibrarySetup = false;

        if (Target.IsInPlatformGroup(UnrealPlatformGroup.Windows))
        {
			ReleaseLib = "oo2core_win64.lib";
			DebugLib = "oo2core_win64_debug.lib";
			PlatformDir = "Win64";
        }
        else if (Target.Platform == UnrealTargetPlatform.Linux)
        {
			ReleaseLib = "liboo2corelinux64.a";
			DebugLib = "liboo2corelinux64_dbg.a";
        }
		else
		{
			// the subclass will return the library names
			ReleaseLib = ReleaseLibraryName;
			DebugLib = DebugLibraryName;
			// platform extensions don't need the Platform directory under lib
			PlatformDir = "";
		}

		if (!bSkipLibrarySetup)
		{
			// combine everything and make sure it was set up properly
			string LibraryToLink = bUseDebugLibrary ? DebugLib : ReleaseLib;
			if (LibraryToLink == null)
			{
				throw new BuildException("Platform {0} doesn't have OodleData libraries properly set up.", Target.Platform);
			}

			PublicAdditionalLibraries.Add(Path.Combine(LibDirectory, PlatformDir, LibraryToLink));
		}
	}
}
