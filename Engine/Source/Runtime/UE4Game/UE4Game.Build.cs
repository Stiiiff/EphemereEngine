// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class UE4Game : ModuleRules
{
	public UE4Game(ReadOnlyTargetRules Target) : base(Target)
	{
		PrivateDependencyModuleNames.Add("Core");

        //DynamicallyLoadedModuleNames.Add("OnlineSubsystemNull");
	}
}
