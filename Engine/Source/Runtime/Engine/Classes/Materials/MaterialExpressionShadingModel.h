// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/EngineTypes.h"
#include "Materials/MaterialExpression.h"
#include "MaterialExpressionShadingModel.generated.h"

/**
 * Compile a select "blend" between ShadingModels
 *
 * @param Compiler				The compiler to add code to
 * @param A						Select A if Alpha is less than 0.5f
 * @param B						Select B if Alpha is greater or equal to 0.5f
 * @param Alpha					Bland factor [0..1]
 * @return						Index to a new code chunk
 */
extern int32 CompileShadingModelBlendFunction(FMaterialCompiler* Compiler, int32 A, int32 B, int32 Alpha);

UCLASS(collapsecategories, hidecategories = Object, MinimalAPI)
class UMaterialExpressionShadingModel : public UMaterialExpression
{
	GENERATED_UCLASS_BODY()

	//~ Begin UMaterialExpression Interface
#if WITH_EDITOR
	virtual int32 Compile(class FMaterialCompiler* Compiler, int32 OutputIndex) override;
	virtual uint32 GetOutputType(int32 OutputIndex) override;
	virtual void GetCaption(TArray<FString>& OutCaptions) const override;
#endif
public:
	UPROPERTY(EditAnywhere, Category=ShadingModel,  meta=(ValidEnumValues="MSM_Lit, MSM_Skin, MSM_Metal, MSM_Foliage, MSM_Hair, MSM_Clothing, MSM_ThickTranslucent"))
	TEnumAsByte<enum EMaterialShadingModel> ShadingModel = MSM_Lit;
	//~ End UMaterialExpression Interface
};
