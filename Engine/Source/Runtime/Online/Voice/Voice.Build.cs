// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.IO;

public class Voice : ModuleRules
{
	public Voice(ReadOnlyTargetRules Target) : base(Target)
	{
		PublicDefinitions.Add("VOICE_PACKAGE=1");

		bool bDontNeedCapture = (Target.Type == TargetType.Server);

		if (bDontNeedCapture)
		{
			PublicDefinitions.Add("VOICE_MODULE_WITH_CAPTURE=0");
		}
		else
		{
			PublicDefinitions.Add("VOICE_MODULE_WITH_CAPTURE=1");
		}

		PublicDependencyModuleNames.AddRange(
			new string[] {
				"Engine",
			}
			);

		PrivateIncludePaths.AddRange(
			new string[] {
				"Runtime/Online/Voice/Private",
			}
			);

		PrivateDependencyModuleNames.AddRange(
			new string[] { 
				"Core",
                "AudioMixer",
				"SignalProcessing"
            }
			);

		AddEngineThirdPartyPrivateStaticDependencies(Target, "SDL2");

		AddEngineThirdPartyPrivateStaticDependencies(Target, "libOpus");
	}
}
