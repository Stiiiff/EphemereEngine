// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class IntelISPC : ModuleRules
{
	public IntelISPC(ReadOnlyTargetRules Target) : base(Target)
	{
		Type = ModuleType.External;

		if (Target.bCompileISPC == true &&
            (Target.WindowsPlatform.StaticAnalyzer != WindowsStaticAnalyzer.PVSStudio &&
            Target.WindowsPlatform.StaticAnalyzer != WindowsStaticAnalyzer.VisualCpp))
        {
	        PublicDefinitions.Add("INTEL_ISPC=1");
        }
		else
        {
            PublicDefinitions.Add("INTEL_ISPC=0");
        }
	}
}