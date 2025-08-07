// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/MaterialMerging.h"
#include "StaticParameterSet.h"
#include "Materials/Material.h"
#include "Engine/Texture2D.h"
#include "MaterialUtilities.h"
#include "Materials/MaterialInstanceConstant.h"
#include "IMeshMergeUtilities.h"
#include "MeshMergeModule.h"
#include "Modules/ModuleManager.h"

namespace ProxyMaterialUtilities
{
#define TEXTURE_MACRO_BASE(a, b, c) \
	bool b##a##Texture = FlattenMaterial.DoesPropertyContainData(EFlattenMaterialProperties::a) && !FlattenMaterial.IsPropertyConstant(EFlattenMaterialProperties::a); \
	if ( b##a##Texture) \
	{ \
		UTexture2D* a##Texture = b##a##Texture ? FMaterialUtilities::CreateTexture(InOuter, AssetBasePath + TEXT("T_") + AssetBaseName + TEXT("_" #a), FlattenMaterial.GetPropertySize(EFlattenMaterialProperties::a), FlattenMaterial.GetPropertySamples(EFlattenMaterialProperties::a), b, TEXTUREGROUP_HierarchicalLOD, RF_Public | RF_Standalone, c) : nullptr; \
		OutMaterial->SetTextureParameterValueEditorOnly(FMaterialParameterInfo(#a "Texture"), a##Texture); \
		FStaticSwitchParameter SwitchParameter; \
		SwitchParameter.ParameterInfo.Name = "Use" #a; \
		SwitchParameter.Value = true; \
		SwitchParameter.bOverride = true; \
		NewStaticParameterSet.StaticSwitchParameters.Add(SwitchParameter); \
		a##Texture->PostEditChange(); \
		OutAssetsToSync.Add(a##Texture); \
	} 

#define TEXTURE_MACRO_VECTOR(a, b, c) TEXTURE_MACRO_BASE(a, b, c)\
	else\
	{ \
		OutMaterial->SetVectorParameterValueEditorOnly(FMaterialParameterInfo(#a "Const"), FlattenMaterial.GetPropertySamples(EFlattenMaterialProperties::a)[0]); \
	} 

#define TEXTURE_MACRO_VECTOR_LINEAR(a, b, c) TEXTURE_MACRO_BASE(a, b, c)\
	else\
	{ \
		OutMaterial->SetVectorParameterValueEditorOnly(FMaterialParameterInfo(#a "Const"), FlattenMaterial.GetPropertySamples(EFlattenMaterialProperties::a)[0].ReinterpretAsLinear()); \
	} 

#define TEXTURE_MACRO_SCALAR(a, b, c) TEXTURE_MACRO_BASE(a, b, c)\
	else \
	{ \
		FLinearColor Colour = FlattenMaterial.IsPropertyConstant(EFlattenMaterialProperties::a) ? FLinearColor::FromSRGBColor( FlattenMaterial.GetPropertySamples(EFlattenMaterialProperties::a)[0]) : FLinearColor( InMaterialProxySettings.a##Constant, 0, 0, 0 ); \
		OutMaterial->SetScalarParameterValueEditorOnly(FMaterialParameterInfo(#a "Const"), Colour.R); \
	}

	static const bool CalculatePackedTextureData(const FFlattenMaterial& InMaterial, bool& bOutPackObjectNormal, bool& bOutPackRoughness, int32& OutNumSamples, FIntPoint& OutSize)
	{
		// Whether or not a material property is baked down
		const bool bHasObjectNormal = InMaterial.DoesPropertyContainData(EFlattenMaterialProperties::ObjectNormal) && !InMaterial.IsPropertyConstant(EFlattenMaterialProperties::ObjectNormal);
		const bool bHasRoughness = InMaterial.DoesPropertyContainData(EFlattenMaterialProperties::Roughness) && !InMaterial.IsPropertyConstant(EFlattenMaterialProperties::Roughness);

		// Check for same texture sizes
		bool bSameTextureSize = false;

		// Determine whether or not the properties sizes match


		const FIntPoint ObjectNormalSize = InMaterial.GetPropertySize(EFlattenMaterialProperties::ObjectNormal);
		const FIntPoint RoughnessSize = InMaterial.GetPropertySize(EFlattenMaterialProperties::Roughness);

		bSameTextureSize = (ObjectNormalSize == RoughnessSize);
		if (bSameTextureSize)
		{
			OutSize = ObjectNormalSize;
			OutNumSamples = InMaterial.GetPropertySamples(EFlattenMaterialProperties::ObjectNormal).Num();
		}
		else
		{
			OutSize = RoughnessSize;
			OutNumSamples = InMaterial.GetPropertySamples(EFlattenMaterialProperties::Roughness).Num();
		}

		// Now that we know if the data matches determine whether or not we should pack the properties
		int32 NumPacked = 0;
		if (OutNumSamples != 0)
		{
			bOutPackObjectNormal = bHasObjectNormal ? (OutNumSamples == InMaterial.GetPropertySamples(EFlattenMaterialProperties::ObjectNormal).Num()) : false;
			NumPacked += (bOutPackObjectNormal) ? 1 : 0;
			bOutPackRoughness = bHasRoughness ? (OutNumSamples == InMaterial.GetPropertySamples(EFlattenMaterialProperties::Roughness).Num()) : false;
			NumPacked += (bOutPackRoughness) ? 1 : 0;
		}
		else
		{
			bOutPackObjectNormal = bOutPackRoughness  = false;
		}

		// Need atleast two properties to pack
		return NumPacked >= 2;
	}

	static UMaterialInstanceConstant* CreateProxyMaterialInstance(UPackage* InOuter, const FMaterialProxySettings& InMaterialProxySettings, UMaterialInterface* InBaseMaterial, const FFlattenMaterial& FlattenMaterial, const FString& AssetBasePath, const FString& AssetBaseName, TArray<UObject*>& OutAssetsToSync)
	{
		UMaterialInterface* BaseMaterial = InBaseMaterial;
		
		const IMeshMergeUtilities& Module = FModuleManager::Get().LoadModuleChecked<IMeshMergeModule>("MeshMergeUtilities").GetUtilities();
		if (!Module.IsValidBaseMaterial(InBaseMaterial, false))
		{
			BaseMaterial = LoadObject<UMaterialInterface>(NULL, TEXT("/Engine/EngineMaterials/BaseFlattenMaterial.BaseFlattenMaterial"), NULL, LOAD_None, NULL);
		} 

		UMaterialInstanceConstant* OutMaterial = FMaterialUtilities::CreateInstancedMaterial(BaseMaterial, InOuter, AssetBasePath + AssetBaseName, RF_Public | RF_Standalone);
		OutAssetsToSync.Add(OutMaterial);

		OutMaterial->BasePropertyOverrides.TwoSided = FlattenMaterial.bTwoSided && InMaterialProxySettings.bAllowTwoSidedMaterial;
		OutMaterial->BasePropertyOverrides.bOverride_TwoSided = (FlattenMaterial.bTwoSided != false) && InMaterialProxySettings.bAllowTwoSidedMaterial;
		OutMaterial->BasePropertyOverrides.DitheredLODTransition = FlattenMaterial.bDitheredLODTransition;
		OutMaterial->BasePropertyOverrides.bOverride_DitheredLODTransition = FlattenMaterial.bDitheredLODTransition != false;

		if (InMaterialProxySettings.BlendMode != BLEND_Opaque)
		{
			OutMaterial->BasePropertyOverrides.bOverride_BlendMode = true;
			OutMaterial->BasePropertyOverrides.BlendMode = InMaterialProxySettings.BlendMode;
		}

		bool bPackObjectNormal, bPackRoughness;
		int32 NumSamples = 0;
		FIntPoint PackedSize;
		const bool bPackTextures = CalculatePackedTextureData(FlattenMaterial, bPackObjectNormal, bPackRoughness, NumSamples, PackedSize);

		const bool bSRGB = true;
		const bool bRGB = false;

		FStaticParameterSet NewStaticParameterSet;

		if(FlattenMaterial.UVChannel != 0)
		{
			// If the used texture coordinate was not the default UV0 set the appropriate one on the instance material
			FStaticSwitchParameter SwitchParameter;
			SwitchParameter.ParameterInfo.Name = TEXT("UseCustomUV");
			SwitchParameter.Value = true;
			SwitchParameter.bOverride = true;
			NewStaticParameterSet.StaticSwitchParameters.Add(SwitchParameter);

			SwitchParameter.ParameterInfo.Name = *(TEXT("UseUV") + FString::FromInt(FlattenMaterial.UVChannel));
			NewStaticParameterSet.StaticSwitchParameters.Add(SwitchParameter);
		}

		// Load textures and set switches accordingly
		if (FlattenMaterial.GetPropertySize(EFlattenMaterialProperties::Diffuse).Num() > 0 && !(FlattenMaterial.IsPropertyConstant(EFlattenMaterialProperties::Diffuse) && FlattenMaterial.GetPropertySamples(EFlattenMaterialProperties::Diffuse)[0] == FColor::Black))
		{
			TEXTURE_MACRO_VECTOR(Diffuse, TC_Default, bSRGB);
		}

		if (FlattenMaterial.GetPropertySize(EFlattenMaterialProperties::Normal).Num() > 1)
		{
			TEXTURE_MACRO_BASE(Normal, TC_Normalmap, bRGB)
		}

		if (FlattenMaterial.GetPropertySize(EFlattenMaterialProperties::ObjectNormal).Num() > 0 && !(FlattenMaterial.IsPropertyConstant(EFlattenMaterialProperties::ObjectNormal) && FlattenMaterial.GetPropertySamples(EFlattenMaterialProperties::ObjectNormal)[0] == FColor::Black))
		{
			TEXTURE_MACRO_VECTOR(ObjectNormal, TC_Default, bRGB);
		}

		// Determine whether or not specific material properties are packed together into one texture (requires at least two to match (number of samples and texture size) in order to be packed

		if (!bPackRoughness && (FlattenMaterial.GetPropertySize(EFlattenMaterialProperties::Roughness).Num() > 0 || !InMaterialProxySettings.bRoughnessMap))
		{
			TEXTURE_MACRO_SCALAR(Roughness, TC_Default, bSRGB);
		}

		const bool bNonSRGB = false;

		if (FlattenMaterial.GetPropertySize(EFlattenMaterialProperties::Opacity).Num() > 0 || !InMaterialProxySettings.bOpacityMap)
		{
			TEXTURE_MACRO_SCALAR(Opacity, TC_Grayscale, bNonSRGB);
		}

		if (FlattenMaterial.GetPropertySize(EFlattenMaterialProperties::OpacityMask).Num() > 0 || !InMaterialProxySettings.bOpacityMaskMap)
		{
			TEXTURE_MACRO_SCALAR(OpacityMask, TC_Grayscale, bNonSRGB);
		}

		if (FlattenMaterial.GetPropertySize(EFlattenMaterialProperties::AmbientOcclusion).Num() > 0 || !InMaterialProxySettings.bAmbientOcclusionMap)
		{
			TEXTURE_MACRO_SCALAR(AmbientOcclusion, TC_Grayscale, bNonSRGB);
		}

		// Handle the packed texture if applicable
		if (bPackTextures)
		{
			TArray<FColor> MergedTexture;
			MergedTexture.AddZeroed(NumSamples);

			// Merge properties into one texture using the separate colour channels
			const EFlattenMaterialProperties Properties[3] = { EFlattenMaterialProperties::ObjectNormal , EFlattenMaterialProperties::Roughness };

			//Property that is not part of the pack (because of a different size), will see is reserve pack space fill with Black Color.
			const bool PropertyShouldBePack[3] = { bPackObjectNormal , bPackRoughness };

			// Red mask (all properties are rendered into the red channel)
			FColor NonAlphaRed = FColor::Red;
			NonAlphaRed.A = 0;
			const uint32 ColorMask = NonAlphaRed.DWColor();
			const uint32 Shift[3] = { 0, 8, 16 };
			for (int32 PropertyIndex = 0; PropertyIndex < 3; ++PropertyIndex)
			{
				const EFlattenMaterialProperties Property = Properties[PropertyIndex];
				const bool HasProperty = PropertyShouldBePack[PropertyIndex] && FlattenMaterial.DoesPropertyContainData(Property) && !FlattenMaterial.IsPropertyConstant(Property);

				if (HasProperty)
				{
					const TArray<FColor>& PropertySamples = FlattenMaterial.GetPropertySamples(Property);
					// OR masked values (samples initialized to zero, so no random data)
					for (int32 SampleIndex = 0; SampleIndex < NumSamples; ++SampleIndex)
					{
						// Black adds the alpha + red channel value shifted into the correct output channel
						MergedTexture[SampleIndex].DWColor() |= (FColor::Black.DWColor() + ((PropertySamples[SampleIndex].DWColor() & ColorMask) >> Shift[PropertyIndex]));
					}
				}
			}

			// Create texture using the merged property data
			UTexture2D* PackedTexture = FMaterialUtilities::CreateTexture(InOuter, AssetBasePath + TEXT("T_") + AssetBaseName + TEXT("_MRS"), PackedSize, MergedTexture, TC_Default, TEXTUREGROUP_HierarchicalLOD, RF_Public | RF_Standalone, bSRGB);
			checkf(PackedTexture, TEXT("Failed to create texture"));
			OutAssetsToSync.Add(PackedTexture);

			// Setup switches for whether or not properties will be packed into one texture
			FStaticSwitchParameter SwitchParameter;
			SwitchParameter.ParameterInfo.Name = TEXT("PackObjectNormal");
			SwitchParameter.Value = bPackObjectNormal;
			SwitchParameter.bOverride = true;
			NewStaticParameterSet.StaticSwitchParameters.Add(SwitchParameter);


			SwitchParameter.ParameterInfo.Name = TEXT("PackRoughness");
			SwitchParameter.Value = bPackRoughness;
			SwitchParameter.bOverride = true;
			NewStaticParameterSet.StaticSwitchParameters.Add(SwitchParameter);

			// Set up switch and texture values
			FMaterialParameterInfo ParameterInfo(TEXT("PackedTexture"));
			OutMaterial->SetTextureParameterValueEditorOnly(ParameterInfo, PackedTexture);
		}

		// Emissive is a special case due to the scaling variable
		if (FlattenMaterial.GetPropertySamples(EFlattenMaterialProperties::Emissive).Num() > 0 && !(FlattenMaterial.GetPropertySize(EFlattenMaterialProperties::Emissive).Num() == 1 && FlattenMaterial.GetPropertySamples(EFlattenMaterialProperties::Emissive)[0] == FColor::Black))
		{
			TEXTURE_MACRO_VECTOR_LINEAR(Emissive, TC_Default, bRGB);

			if (FlattenMaterial.EmissiveScale != 1.0f)
			{
				FMaterialParameterInfo ParameterInfo(TEXT("EmissiveScale"));
				OutMaterial->SetScalarParameterValueEditorOnly(ParameterInfo, FlattenMaterial.EmissiveScale);
			}
		}

		// Force initializing the static permutations according to the switches we have set
		OutMaterial->UpdateStaticPermutation(NewStaticParameterSet);
		OutMaterial->InitStaticPermutation();

		OutMaterial->PostEditChange();

		return OutMaterial;
	}	

	static UMaterialInstanceConstant* CreateProxyMaterialInstance(UPackage* InOuter, const FMaterialProxySettings& InMaterialProxySettings, FFlattenMaterial& FlattenMaterial, const FString& AssetBasePath, const FString& AssetBaseName, TArray<UObject*>& OutAssetsToSync)
	{
		UMaterialInterface* BaseMaterial = LoadObject<UMaterialInterface>(NULL, TEXT("/Engine/EngineMaterials/BaseFlattenMaterial.BaseFlattenMaterial"), NULL, LOAD_None, NULL);
		return CreateProxyMaterialInstance(InOuter, InMaterialProxySettings, BaseMaterial, FlattenMaterial, AssetBasePath, AssetBasePath, OutAssetsToSync);
	}

};
