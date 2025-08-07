// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.Collections.Generic;

[SupportedPlatforms(UnrealPlatformClass.All)]
public class UE4GameTarget : TargetRules
{
	public UE4GameTarget( TargetInfo Target ) : base(Target)
	{
		Type = TargetType.Game;
		BuildEnvironment = TargetBuildEnvironment.Shared;

		ExtraModuleNames.Add("UE4Game");
	}
}
