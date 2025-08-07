// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "GameFramework/Actor.h"
#include "Light.generated.h"

class UMaterialInterface;

UCLASS(Abstract, ClassGroup=Lights, hideCategories=(Input,Collision,Replication), showCategories=("Input|MouseInput"), ComponentWrapperClass, ConversionRoot, meta=(ChildCanTick))
class ENGINE_API ALight : public AActor
{
	GENERATED_UCLASS_BODY()

private:
	/** @todo document */
	UPROPERTY(Category = Light, BlueprintReadOnly, meta = (ExposeFunctionCategories = "Light,Rendering,Rendering|Components|Light", AllowPrivateAccess = "true"))
	class ULightComponent* LightComponent;
public:

	virtual void GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const override;

	/** replicated copy of LightComponent's bEnabled property */
	UPROPERTY(replicatedUsing=OnRep_bEnabled)
	uint32 bEnabled:1;

	/** Replication Notification Callbacks */
	UFUNCTION()
	virtual void OnRep_bEnabled();

	/** Function to change mobility type of light */
	void SetMobility(EComponentMobility::Type InMobility);

public:
#if WITH_EDITOR
	//~ Begin AActor Interface.
	virtual void CheckForErrors() override;
	//~ End AActor Interface.
#endif

	/**
	 * Return whether the light supports being toggled off and on on-the-fly.
	 */
	bool IsToggleable() const;

	//~ Begin AActor Interface.
	virtual void Destroyed() override;
	virtual bool IsLevelBoundsRelevant() const override { return false; }
	//~ End AActor Interface.

	/** Returns LightComponent subobject **/
	class ULightComponent* GetLightComponent() const { return LightComponent; }
};



