// Copyright Epic Games, Inc. All Rights Reserved.

using System.IO;
using System.Collections.Generic;
using UnrealBuildTool;

public class OodleNetworkHandlerComponent : ModuleRules 
{
	protected virtual string OodleVersion { get { return "2.9.0"; } }

	// Platform Extensions need to override these
	protected virtual string LibRootDirectory { get { return ModuleDirectory; } }
	protected virtual string ReleaseLibraryName { get { return null; } }
	protected virtual string DebugLibraryName { get { return null; } }

	protected virtual string SdkBaseDirectory { get { return Path.Combine(LibRootDirectory, "..", "Sdks", OodleVersion); } }
	protected virtual string LibDirectory { get { return Path.Combine(SdkBaseDirectory, "lib"); } }

	protected virtual string IncludeDirectory { get { return Path.Combine(ModuleDirectory, "..", "Sdks", OodleVersion, "include"); } }


	public OodleNetworkHandlerComponent(ReadOnlyTargetRules Target) : base(Target)
	{
		ShortName = "OodleNetworkPlugin";

		PublicIncludePaths.Add(IncludeDirectory);

		PublicDependencyModuleNames.Add("PacketHandler");
		PublicDependencyModuleNames.Add("Core");
		PublicDependencyModuleNames.Add("CoreUObject");
		PublicDependencyModuleNames.Add("NetCore");
		PublicDependencyModuleNames.Add("Engine");
		PublicDependencyModuleNames.Add("Analytics");

		PrivatePCHHeaderFile = "Private/OodleNetworkHandlerComponentPCH.h";

		// We depend on the .udic files, otherwise those dictionaries don't pass-through to final builds!
		RuntimeDependencies.Add("$(ProjectDir)/Content/Oodle/...", StagedFileType.UFS);

		string ReleaseLib = null;
		string DebugLib = null;
		string PlatformDir = Target.Platform.ToString();

		bool bAllowDebugLibrary = false;
		bool bUseDebugLibrary = bAllowDebugLibrary && Target.Configuration == UnrealTargetConfiguration.Debug && Target.bDebugBuildsActuallyUseDebugCRT;

		bool bSkipLibrarySetup = false;

        if (Target.IsInPlatformGroup(UnrealPlatformGroup.Windows))
		{
			ReleaseLib = "oo2net_win64.lib";
			DebugLib = "oo2net_win64_debug.lib";
			PlatformDir = "Win64";
		}
		else if (Target.Platform == UnrealTargetPlatform.Linux)
		{
			ReleaseLib = "liboo2netlinux64.a";
			DebugLib = "liboo2netlinux64_dbg.a";
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
