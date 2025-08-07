// Copyright Epic Games, Inc. All Rights Reserved.


#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "PhysicsEngine/RigidBodyBase.h"
#include "RadialForceActor.generated.h"

class UBillboardComponent;

UCLASS(MinimalAPI, hideCategories=(Collision, Input), showCategories=("Input|MouseInput"), ComponentWrapperClass)
class ARadialForceActor : public ARigidBodyBase
{
	GENERATED_UCLASS_BODY()

private:
	/** Force component */
	UPROPERTY(Category = RadialForceActor, VisibleAnywhere, BlueprintReadOnly, meta = (ExposeFunctionCategories = "Activation,Components|Activation,Physics,Physics|Components|RadialForce", AllowPrivateAccess = "true"))
	class URadialForceComponent* ForceComponent;

#if WITH_EDITORONLY_DATA
	UPROPERTY()
	UBillboardComponent* SpriteComponent;
#endif
public:

#if WITH_EDITOR
	//~ Begin AActor Interface.
	virtual void EditorApplyScale(const FVector& DeltaScale, const FVector* PivotLocation, bool bAltDown, bool bShiftDown, bool bCtrlDown) override;
	//~ End AActor Interface.
#endif

public:
	/** Returns ForceComponent subobject **/
	ENGINE_API class URadialForceComponent* GetForceComponent() const { return ForceComponent; }
#if WITH_EDITORONLY_DATA
	/** Returns SpriteComponent subobject **/
	ENGINE_API UBillboardComponent* GetSpriteComponent() const { return SpriteComponent; }
#endif
};



