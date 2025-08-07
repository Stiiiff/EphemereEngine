// Copyright Epic Games, Inc. All Rights Reserved.
//
#include "MaterialStatsCommon.h"
#include "EngineGlobals.h"
#include "MaterialStats.h"
#include "LocalVertexFactory.h"
#include "GPUSkinVertexFactory.h"
#include "MaterialEditorSettings.h"
#include "RHIShaderFormatDefinitions.inl"
#include "ShaderCompilerCore.h"

/***********************************************************************************************************************/
/*begin FMaterialResourceStats functions*/

void FMaterialResourceStats::SetupExtaCompilationSettings(const EShaderPlatform Platform, FExtraShaderCompilerSettings& Settings) const
{
	Settings.bExtractShaderSource = true;
	Settings.OfflineCompilerPath = FMaterialStatsUtils::GetPlatformOfflineCompilerPath(Platform);
}

/*end FMaterialResourceStats functions*/
/***********************************************************************************************************************/

/***********************************************************************************************************************/
/*begin FMaterialStatsUtils */
const FLinearColor FMaterialStatsUtils::BlueColor(0.1851f, 1.0f, 0.940258f);
const FLinearColor FMaterialStatsUtils::YellowColor(1.0f, 0.934216f, 0.199542f);
const FLinearColor FMaterialStatsUtils::GreenColor(0.540805f, 1.0f, 0.321716f);
const FLinearColor FMaterialStatsUtils::OrangeColor(1.0f, 0.316738f, 0.095488f);
const FLinearColor FMaterialStatsUtils::DefaultGridTextColor(0.244819f, 0.301351f, 0.390625f);

TSharedPtr<FMaterialStats> FMaterialStatsUtils::CreateMaterialStats(class IMaterialEditor* MaterialEditor)
{
	TSharedPtr<FMaterialStats> MaterialStats = MakeShareable(new FMaterialStats());
	MaterialStats->Initialize(MaterialEditor);

	return MaterialStats;
}

FString FMaterialStatsUtils::MaterialQualityToString(const EMaterialQualityLevel::Type Quality)
{
	FString StrQuality;

	switch (Quality)
	{
		case EMaterialQualityLevel::High:
			StrQuality = TEXT("High Quality");
		break;
		case EMaterialQualityLevel::Medium:
			StrQuality = TEXT("Medium Quality");
		break;
		case EMaterialQualityLevel::Low:
			StrQuality = TEXT("Low Quality");
		break;
		case EMaterialQualityLevel::Epic:
			StrQuality = TEXT("Epic Quality");
		break;
	}

	return StrQuality;
}

FString FMaterialStatsUtils::MaterialQualityToShortString(const EMaterialQualityLevel::Type Quality)
{
	FString StrQuality;

	switch (Quality)
	{
		case EMaterialQualityLevel::High:
			StrQuality = TEXT("High");
		break;
		case EMaterialQualityLevel::Medium:
			StrQuality = TEXT("Medium");
		break;
		case EMaterialQualityLevel::Low:
			StrQuality = TEXT("Low");
		break;
		case EMaterialQualityLevel::Epic:
			StrQuality = TEXT("Epic");
		break;
	}

	return StrQuality;
}

EMaterialQualityLevel::Type FMaterialStatsUtils::StringToMaterialQuality(const FString& StrQuality)
{
	if (StrQuality.Equals(TEXT("High Quality")))
	{
		return EMaterialQualityLevel::High;
	}		
	else if (StrQuality.Equals(TEXT("Medium Quality")))
	{
		return EMaterialQualityLevel::Medium;
	}
	else if (StrQuality.Equals(TEXT("Low Quality")))
	{
		return EMaterialQualityLevel::Low;
	}
	else if (StrQuality.Equals(TEXT("Epic Quality")))
	{
		return EMaterialQualityLevel::Epic;
	}

	return EMaterialQualityLevel::Num;
}

FString FMaterialStatsUtils::GetPlatformTypeName(const EPlatformCategoryType InEnumValue)
{
	FString PlatformName;

	switch (InEnumValue)
	{
		case EPlatformCategoryType::Desktop:
			PlatformName = FString("Desktop");
		break;
	}

	return PlatformName;
}

FString FMaterialStatsUtils::ShaderPlatformTypeName(const EShaderPlatform PlatformID)
{
	switch (PlatformID)
	{
		case SP_VULKAN_SM5:
			return FString("VULKAN_SM5");
		default:
			FString FormatName = ShaderPlatformToShaderFormatName(PlatformID).ToString();
			if (FormatName.StartsWith(TEXT("SF_")))
			{
				FormatName.MidInline(3, MAX_int32, false);
			}
			return FormatName;
	}
}

FString FMaterialStatsUtils::GetPlatformOfflineCompilerPath(const EShaderPlatform ShaderPlatform)
{
	return FString();
}

bool FMaterialStatsUtils::IsPlatformOfflineCompilerAvailable(const EShaderPlatform ShaderPlatform)
{
	FString CompilerPath = GetPlatformOfflineCompilerPath(ShaderPlatform);

	bool bCompilerExists = FPaths::FileExists(CompilerPath);

	return bCompilerExists;
}

bool FMaterialStatsUtils::PlatformNeedsOfflineCompiler(const EShaderPlatform ShaderPlatform)
{
	switch (ShaderPlatform)
	{
		case SP_VULKAN_SM5:
			return true;

		default:
			return FDataDrivenShaderPlatformInfo::GetNeedsOfflineCompiler(ShaderPlatform);
	}

	return false;
}

FString FMaterialStatsUtils::RepresentativeShaderTypeToString(const ERepresentativeShader ShaderType)
{
	switch (ShaderType)
	{
		case ERepresentativeShader::StationarySurface:
			return TEXT("Stationary surface");
		break;

		case ERepresentativeShader::StationarySurfaceCSM:
			return TEXT("Stationary surface + CSM");
		break;

		case ERepresentativeShader::StationarySurface1PointLight:
		case ERepresentativeShader::StationarySurfaceNPointLights:
			return TEXT("Stationary surface + Point Lights");
		break;

		case ERepresentativeShader::DynamicallyLitObject:
			return TEXT("Dynamically lit object");
		break;

		case ERepresentativeShader::StaticMesh:
			return TEXT("Static Mesh");
		break;

		case ERepresentativeShader::SkeletalMesh:
			return TEXT("Skeletal Mesh");
		break;

		case ERepresentativeShader::UIDefaultFragmentShader:
			return TEXT("UI Pixel Shader");
		break;

		case ERepresentativeShader::UIDefaultVertexShader:
			return TEXT("UI Vertex Shader");
		break;

		case ERepresentativeShader::UIInstancedVertexShader:
			return TEXT("UI Instanced Vertex Shader");
		break;

		default:
			return TEXT("Unknown shader name");
		break;
	}
}

FLinearColor FMaterialStatsUtils::PlatformTypeColor(EPlatformCategoryType PlatformType)
{
	FLinearColor Color(FLinearColor::Blue);

	switch (PlatformType)
	{
		case EPlatformCategoryType::Desktop:
			Color = BlueColor;
		break;
		default:
			return Color;
		break;
	}

	return Color;
}

FLinearColor FMaterialStatsUtils::QualitySettingColor(const EMaterialQualityLevel::Type QualityType)
{
	switch (QualityType)
	{
		case EMaterialQualityLevel::Low:
			return GreenColor;
		break;
		case EMaterialQualityLevel::High:
			return OrangeColor;
		break;
		case EMaterialQualityLevel::Medium:
			return YellowColor;
		break;
		case EMaterialQualityLevel::Epic:
			return FLinearColor::Red;
		break;
		default:
			return FLinearColor::Black;
		break;
	}

	return FLinearColor::Black;
}

void FMaterialStatsUtils::GetRepresentativeShaderTypesAndDescriptions(TMap<FName, TArray<FRepresentativeShaderInfo>>& ShaderTypeNamesAndDescriptions, const FMaterial* TargetMaterial)
{
	static const FName FLocalVertexFactoryName = FLocalVertexFactory::StaticType.GetFName();
	static const FName FGPUFactoryName = TEXT("TGPUSkinVertexFactoryDefault");

	if (TargetMaterial->IsUIMaterial())
	{
		static FName TSlateMaterialShaderPSDefaultName = TEXT("TSlateMaterialShaderPSDefault");
		ShaderTypeNamesAndDescriptions.FindOrAdd(FLocalVertexFactoryName)
			.Add(FRepresentativeShaderInfo(ERepresentativeShader::UIDefaultFragmentShader, TSlateMaterialShaderPSDefaultName, TEXT("Default UI Pixel Shader")));

		static FName TSlateMaterialShaderVSfalseName = TEXT("TSlateMaterialShaderVSfalse");
		ShaderTypeNamesAndDescriptions.FindOrAdd(FLocalVertexFactoryName)
			.Add(FRepresentativeShaderInfo(ERepresentativeShader::UIDefaultVertexShader, TSlateMaterialShaderVSfalseName, TEXT("Default UI Vertex Shader")));

		static FName TSlateMaterialShaderVStrueName = TEXT("TSlateMaterialShaderVStrue");
		ShaderTypeNamesAndDescriptions.FindOrAdd(FLocalVertexFactoryName)
			.Add(FRepresentativeShaderInfo(ERepresentativeShader::UIInstancedVertexShader, TSlateMaterialShaderVStrueName, TEXT("Instanced UI Vertex Shader")));
	}
	else if (TargetMaterial->GetFeatureLevel() >= ERHIFeatureLevel::SM5)
	{
		if (TargetMaterial->GetShadingModels().IsUnlit())
		{
			//unlit materials are never lightmapped
			static FName TBasePassPSFNoLightMapPolicyName = TEXT("TBasePassPSFNoLightMapPolicy");
			ShaderTypeNamesAndDescriptions.FindOrAdd(FLocalVertexFactoryName)
				.Add(FRepresentativeShaderInfo(ERepresentativeShader::StationarySurface, TBasePassPSFNoLightMapPolicyName, TEXT("Base pass shader without light map")));
		}
		else
		{
			//also show a dynamically lit shader
			static FName TBasePassPSFNoLightMapPolicyName = TEXT("TBasePassPSFNoLightMapPolicy");
			ShaderTypeNamesAndDescriptions.FindOrAdd(FLocalVertexFactoryName)
				.Add(FRepresentativeShaderInfo(ERepresentativeShader::DynamicallyLitObject, TBasePassPSFNoLightMapPolicyName, TEXT("Base pass shader")));

			static auto* CVarAllowStaticLighting = IConsoleManager::Get().FindTConsoleVariableDataInt(TEXT("r.AllowStaticLighting"));
			const bool bAllowStaticLighting = CVarAllowStaticLighting->GetValueOnAnyThread() != 0;

			if (bAllowStaticLighting)
			{
				if (TargetMaterial->IsUsedWithStaticLighting())
				{
					static FName TBasePassPSTLightMapPolicyName = TEXT("TBasePassPSTDistanceFieldShadowsAndLightMapPolicyHQ");
					ShaderTypeNamesAndDescriptions.FindOrAdd(FLocalVertexFactoryName)
						.Add(FRepresentativeShaderInfo(ERepresentativeShader::StationarySurface, TBasePassPSTLightMapPolicyName, TEXT("Base pass shader with Surface Lightmap")));
				}

				static FName TBasePassPSFPrecomputedVolumetricLightmapLightingPolicyName = TEXT("TBasePassPSFPrecomputedVolumetricLightmapLightingPolicy");
				ShaderTypeNamesAndDescriptions.FindOrAdd(FLocalVertexFactoryName)
					.Add(FRepresentativeShaderInfo(ERepresentativeShader::DynamicallyLitObject, TBasePassPSFPrecomputedVolumetricLightmapLightingPolicyName, TEXT("Base pass shader with Volumetric Lightmap")));
			}
		}

		static FName TBasePassVSFNoLightMapPolicyName = TEXT("TBasePassVSFNoLightMapPolicy");
		ShaderTypeNamesAndDescriptions.FindOrAdd(FLocalVertexFactoryName)
			.Add(FRepresentativeShaderInfo(ERepresentativeShader::StaticMesh, TBasePassVSFNoLightMapPolicyName, TEXT("Base pass vertex shader")));

		ShaderTypeNamesAndDescriptions.FindOrAdd(FGPUFactoryName)
			.Add(FRepresentativeShaderInfo(ERepresentativeShader::SkeletalMesh, TBasePassVSFNoLightMapPolicyName, TEXT("Base pass vertex shader")));
	}
}

/**
* Gets instruction counts that best represent the likely usage of this material based on shading model and other factors.
* @param Results - an array of descriptions to be populated
*/
void FMaterialStatsUtils::GetRepresentativeInstructionCounts(TArray<FShaderInstructionsInfo>& Results, const FMaterialResource* Target)
{
	TMap<FName, TArray<FRepresentativeShaderInfo>> ShaderTypeNamesAndDescriptions;
	Results.Empty();

	//when adding a shader type here be sure to update FPreviewMaterial::ShouldCache()
	//so the shader type will get compiled with preview materials
	const FMaterialShaderMap* MaterialShaderMap = Target->GetGameThreadShaderMap();
	if (MaterialShaderMap && MaterialShaderMap->IsCompilationFinalized())
	{
		GetRepresentativeShaderTypesAndDescriptions(ShaderTypeNamesAndDescriptions, Target);

		if (Target->IsUIMaterial())
		{
			//for (const TPair<FName, FRepresentativeShaderInfo>& ShaderTypePair : ShaderTypeNamesAndDescriptions)
			for (auto DescriptionPair : ShaderTypeNamesAndDescriptions)
			{
				auto& DescriptionArray = DescriptionPair.Value;
				for (int32 i = 0; i < DescriptionArray.Num(); ++i)
				{
					const FRepresentativeShaderInfo& ShaderInfo = DescriptionArray[i];

					FShaderType* ShaderType = FindShaderTypeByName(ShaderInfo.ShaderName);
					check(ShaderType);
					const int32 NumInstructions = MaterialShaderMap->GetMaxNumInstructionsForShader(ShaderType);

					FShaderInstructionsInfo Info;
					Info.ShaderType = ShaderInfo.ShaderType;
					Info.ShaderDescription = ShaderInfo.ShaderDescription;
					Info.InstructionCount = NumInstructions;

					Results.Push(Info);
				}
			}
		}
		else
		{
			for (auto DescriptionPair : ShaderTypeNamesAndDescriptions)
			{
				FVertexFactoryType* FactoryType = FindVertexFactoryType(DescriptionPair.Key);
				const FMeshMaterialShaderMap* MeshShaderMap = MaterialShaderMap->GetMeshShaderMap(FactoryType);
				if (MeshShaderMap)
				{
					TMap<FHashedName, TShaderRef<FShader>> ShaderMap;
					MeshShaderMap->GetShaderList(*MaterialShaderMap, ShaderMap);

					auto& DescriptionArray = DescriptionPair.Value;

					for (int32 i = 0; i < DescriptionArray.Num(); ++i)
					{
						const FRepresentativeShaderInfo& ShaderInfo = DescriptionArray[i];

						TShaderRef<FShader>* ShaderEntry = ShaderMap.Find(ShaderInfo.ShaderName);
						if (ShaderEntry != nullptr)
						{
							FShaderType* ShaderType = (*ShaderEntry).GetType();
							{
								const int32 NumInstructions = MeshShaderMap->GetMaxNumInstructionsForShader(*MaterialShaderMap, ShaderType);

								FShaderInstructionsInfo Info;
								Info.ShaderType = ShaderInfo.ShaderType;
								Info.ShaderDescription = ShaderInfo.ShaderDescription;
								Info.InstructionCount = NumInstructions;

								Results.Push(Info);
							}
						}
					}
				}
			}
		}
	}
}

void FMaterialStatsUtils::ExtractMatertialStatsInfo(FShaderStatsInfo& OutInfo, const FMaterialResource* MaterialResource)
{
	// extract potential errors
	const ERHIFeatureLevel::Type MaterialFeatureLevel = MaterialResource->GetFeatureLevel();
	FString FeatureLevelName;
	GetFeatureLevelName(MaterialFeatureLevel, FeatureLevelName);

	OutInfo.Empty();
	TArray<FString> CompileErrors = MaterialResource->GetCompileErrors();
	for (int32 ErrorIndex = 0; ErrorIndex < CompileErrors.Num(); ErrorIndex++)
	{
		OutInfo.StrShaderErrors += FString::Printf(TEXT("[%s] %s\n"), *FeatureLevelName, *CompileErrors[ErrorIndex]);
	}

	bool bNoErrors = OutInfo.StrShaderErrors.Len() == 0;

	if (bNoErrors)
	{
		// extract instructions info
		TArray<FMaterialStatsUtils::FShaderInstructionsInfo> ShaderInstructionInfo;
		GetRepresentativeInstructionCounts(ShaderInstructionInfo, MaterialResource);

		for (int32 InstructionIndex = 0; InstructionIndex < ShaderInstructionInfo.Num(); InstructionIndex++)
		{
			FShaderStatsInfo::FContent Content;

			Content.StrDescription = ShaderInstructionInfo[InstructionIndex].InstructionCount > 0 ? FString::Printf(TEXT("%u"), ShaderInstructionInfo[InstructionIndex].InstructionCount) : TEXT("n/a");
			Content.StrDescriptionLong = ShaderInstructionInfo[InstructionIndex].InstructionCount > 0 ?
				FString::Printf(TEXT("%s: %u instructions"), *ShaderInstructionInfo[InstructionIndex].ShaderDescription, ShaderInstructionInfo[InstructionIndex].InstructionCount) :
				TEXT("Offline shader compiler not available or an error was encountered!");

			OutInfo.ShaderInstructionCount.Add(ShaderInstructionInfo[InstructionIndex].ShaderType, Content);
		}

		// extract samplers info
		const int32 SamplersUsed = FMath::Max(MaterialResource->GetSamplerUsage(), 0);
		const int32 MaxSamplers = GetExpectedFeatureLevelMaxTextureSamplers(MaterialResource->GetFeatureLevel());
		OutInfo.SamplersCount.StrDescription = FString::Printf(TEXT("%u/%u"), SamplersUsed, MaxSamplers);
		OutInfo.SamplersCount.StrDescriptionLong = FString::Printf(TEXT("%s samplers: %u/%u"), TEXT("Texture"), SamplersUsed, MaxSamplers);

		// extract esimated sample info
		uint32 NumVSTextureSamples = 0, NumPSTextureSamples = 0;
		MaterialResource->GetEstimatedNumTextureSamples(NumVSTextureSamples, NumPSTextureSamples);

		OutInfo.TextureSampleCount.StrDescription = FString::Printf(TEXT("VS(%u), PS(%u)"), NumVSTextureSamples, NumPSTextureSamples);
		OutInfo.TextureSampleCount.StrDescriptionLong = FString::Printf(TEXT("Texture Lookups (Est.): Vertex(%u), Pixel(%u)"), NumVSTextureSamples, NumPSTextureSamples);

		// extract estimated VT info
		const uint32 NumVirtualTextureLookups = MaterialResource->GetEstimatedNumVirtualTextureLookups();
		OutInfo.VirtualTextureLookupCount.StrDescription = FString::Printf(TEXT("%u"), NumVirtualTextureLookups);
		OutInfo.VirtualTextureLookupCount.StrDescriptionLong = FString::Printf(TEXT("Virtual Texture Lookups (Est.): %u"), NumVirtualTextureLookups);

		const uint32 NumVirtualTextureStacks = MaterialResource->GetNumVirtualTextureStacks();
		OutInfo.VirtualTextureStackCount.StrDescription = FString::Printf(TEXT("%u"), NumVirtualTextureStacks);
		OutInfo.VirtualTextureStackCount.StrDescriptionLong = FString::Printf(TEXT("Virtual Texture Stacks (Est.): %u"), NumVirtualTextureStacks);

		// extract interpolators info
		uint32 UVScalarsUsed, CustomInterpolatorScalarsUsed;
		MaterialResource->GetUserInterpolatorUsage(UVScalarsUsed, CustomInterpolatorScalarsUsed);

		const uint32 TotalScalars = UVScalarsUsed + CustomInterpolatorScalarsUsed;
		const uint32 MaxScalars = FMath::DivideAndRoundUp(TotalScalars, 4u) * 4;

		OutInfo.InterpolatorsCount.StrDescription = FString::Printf(TEXT("%u/%u"), TotalScalars, MaxScalars);
		OutInfo.InterpolatorsCount.StrDescriptionLong = FString::Printf(TEXT("User interpolators: %u/%u Scalars (%u/4 Vectors) (TexCoords: %i, Custom: %i)"),
			TotalScalars, MaxScalars, MaxScalars / 4, UVScalarsUsed, CustomInterpolatorScalarsUsed);
	}
}

/*end FMaterialStatsUtils */
/***********************************************************************************************************************/
