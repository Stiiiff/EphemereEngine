// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.IO;

public class heapprofd : ModuleRules
{
	public heapprofd(ReadOnlyTargetRules Target) : base(Target)
	{
		Type = ModuleType.External;

        string BasePath = Target.UEThirdPartySourceDirectory + "heapprofd";
		PublicSystemIncludePaths.Add(BasePath);
    }
}
