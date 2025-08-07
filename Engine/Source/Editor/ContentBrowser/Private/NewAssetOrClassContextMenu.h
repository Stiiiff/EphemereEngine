// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Framework/Commands/UIAction.h"
#include "Templates/SharedPointer.h"

class FMenuBuilder;
class UToolMenu;

class FNewAssetOrClassContextMenu
{
public:
	DECLARE_DELEGATE_OneParam( FOnNewFolderRequested, const FString& /*SelectedPath*/ );

	/** Makes the context menu widget */
	static void MakeContextMenu(
		UToolMenu* Menu, 
		const TArray<FName>& InSelectedPaths, 
		const FOnNewFolderRequested& InOnNewFolderRequested
		);

private:
	/** Create a new folder at the specified path */
	static void ExecuteNewFolder(FName InPath, FOnNewFolderRequested InOnNewFolderRequested);
};
