// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Engine/DataAsset.h"
#include "ConcertAssetContainer.generated.h"


// Forward declarations
class USoundBase;
class USoundCue;
class UStaticMesh;
class UMaterial;
class UMaterialInterface;
class UMaterialInstance;
class UFont;

/**
 * Asset container for VREditor.
 */
UCLASS()
class CONCERTSYNCCLIENT_API UConcertAssetContainer : public UDataAsset
{
	GENERATED_BODY()

public:

	//
	// Meshes
	//

	UPROPERTY(EditAnywhere, Category = Mesh)
	UStaticMesh* GenericDesktopMesh;

	//
	// Materials
	//

	UPROPERTY(EditAnywhere, Category = Material)
	UMaterialInterface* PresenceMaterial;

	UPROPERTY(EditAnywhere, Category = Material)
	UMaterialInterface* TextMaterial;

	UPROPERTY(EditAnywhere, Category = Material)
	UMaterialInterface* LaserCoreMaterial;

	UPROPERTY(EditAnywhere, Category = Material)
	UMaterialInterface* LaserMaterial;

	UPROPERTY(EditAnywhere, Category = Material)
	UMaterialInterface* PresenceFadeMaterial;
};
