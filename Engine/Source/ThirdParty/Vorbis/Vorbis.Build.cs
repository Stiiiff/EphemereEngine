// Copyright Epic Games, Inc. All Rights Reserved.
using UnrealBuildTool;
using System.IO;

public class Vorbis : ModuleRules
{
	protected virtual string VorbisVersion { get { return "libvorbis-1.3.2"; } }
	protected virtual string IncRootDirectory { get { return Target.UEThirdPartySourceDirectory; } }
	protected virtual string LibRootDirectory { get { return Target.UEThirdPartySourceDirectory; } }

	protected virtual string VorbisIncPath { get { return Path.Combine(IncRootDirectory, "Vorbis", VorbisVersion, "include"); } }
	protected virtual string VorbisLibPath { get { return Path.Combine(LibRootDirectory, "Vorbis", VorbisVersion, "lib"); } }

	public Vorbis(ReadOnlyTargetRules Target) : base(Target)
	{
		Type = ModuleType.External;

		PublicIncludePaths.Add(VorbisIncPath);
		PublicDefinitions.Add("WITH_OGGVORBIS=1");

		if (Target.Platform == UnrealTargetPlatform.Win64)
		{
			PublicAdditionalLibraries.Add(Path.Combine(VorbisLibPath, "win64", "VS" + Target.WindowsPlatform.GetVisualStudioCompilerVersionName(), "libvorbis_64.lib"));
			PublicDelayLoadDLLs.Add("libvorbis_64.dll");

			RuntimeDependencies.Add("$(EngineDir)/Binaries/ThirdParty/Vorbis/Win64/VS" + Target.WindowsPlatform.GetVisualStudioCompilerVersionName() + "/libvorbis_64.dll");
		}
		else if (Target.IsInPlatformGroup(UnrealPlatformGroup.Unix))
		{
			PublicAdditionalLibraries.Add(Path.Combine(VorbisLibPath, "Linux", Target.Architecture, "libvorbis.a"));
		}
	}
}
