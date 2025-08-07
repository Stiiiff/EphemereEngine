// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "GameFramework/Actor.h"
#include "AmbientSound.generated.h"

/** A sound actor that can be placed in a level */
UCLASS(AutoExpandCategories=Audio, ClassGroup=Sounds, hideCategories(Collision, Input, Game), showCategories=("Input|MouseInput", "Game|Damage"), ComponentWrapperClass)
class ENGINE_API AAmbientSound : public AActor
{
	GENERATED_UCLASS_BODY()

private:
	/** Audio component that handles sound playing */
	UPROPERTY(Category = Sound, VisibleAnywhere, BlueprintReadOnly, meta = (ExposeFunctionCategories = "Sound,Audio,Audio|Components|Audio", AllowPrivateAccess = "true"))
	class UAudioComponent* AudioComponent;
public:
	
	FString GetInternalSoundCueName();

	//~ Begin AActor Interface.
#if WITH_EDITOR
	virtual void CheckForErrors() override;
	virtual bool GetReferencedContentObjects( TArray<UObject*>& Objects ) const override;
#endif
	virtual void PostRegisterAllComponents() override;
	//~ End AActor Interface.

public:
	/** Returns AudioComponent subobject **/
	class UAudioComponent* GetAudioComponent() const { return AudioComponent; }
};



