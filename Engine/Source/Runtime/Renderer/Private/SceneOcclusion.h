// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Shader.h"
#include "GlobalShader.h"
#include "ShadowRendering.h"
#include "Engine/Engine.h"

class FProjectedShadowInfo;
class FPlanarReflectionSceneProxy;
class FRHIRenderQuery;

DECLARE_GPU_STAT_NAMED_EXTERN(HZB, TEXT("HZB"));

/*=============================================================================
	SceneOcclusion.h
=============================================================================*/

struct FViewOcclusionQueries
{
	using FProjectedShadowArray = TArray<FProjectedShadowInfo const*, SceneRenderingAllocator>;
	using FPlanarReflectionArray = TArray<FPlanarReflectionSceneProxy const*, SceneRenderingAllocator>;
	using FRenderQueryArray = TArray<FRHIRenderQuery*, SceneRenderingAllocator>;

	FProjectedShadowArray PointLightQueryInfos;
	FProjectedShadowArray CSMQueryInfos;
	FProjectedShadowArray ShadowQuerieInfos;
	FPlanarReflectionArray ReflectionQuerieInfos;

	FRenderQueryArray PointLightQueries;
	FRenderQueryArray CSMQueries;
	FRenderQueryArray ShadowQueries;
	FRenderQueryArray ReflectionQueries;

	bool bFlushQueries = true;
};

using FViewOcclusionQueriesPerView = TArray<FViewOcclusionQueries, TInlineAllocator<1, SceneRenderingAllocator>>;

/**
* A vertex shader for rendering a texture on a simple element.
*/
class FOcclusionQueryVS : public FGlobalShader
{
	DECLARE_SHADER_TYPE(FOcclusionQueryVS,Global);
public:
	static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters) { return IsFeatureLevelSupported(Parameters.Platform, ERHIFeatureLevel::SM5); }

	static void ModifyCompilationEnvironment(const FGlobalShaderPermutationParameters& Parameters, FShaderCompilerEnvironment& OutEnvironment)
	{
		FGlobalShader::ModifyCompilationEnvironment(Parameters, OutEnvironment);
		OutEnvironment.SetDefine(TEXT("OUTPUT_GAMMA_SPACE"), 1);
	}

	FOcclusionQueryVS(const ShaderMetaType::CompiledShaderInitializerType& Initializer):
		FGlobalShader(Initializer)
	{
			StencilingGeometryParameters.Bind(Initializer.ParameterMap);
			ViewId.Bind(Initializer.ParameterMap, TEXT("ViewId"));
	}

	FOcclusionQueryVS() {}

	void SetParametersWithBoundingSphere(FRHICommandList& RHICmdList, const FViewInfo& View, const FSphere& BoundingSphere)
	{
		FGlobalShader::SetParameters<FViewUniformShaderParameters>(RHICmdList, RHICmdList.GetBoundVertexShader(), View.ViewUniformBuffer);

		FVector4 StencilingSpherePosAndScale;
		StencilingGeometry::GStencilSphereVertexBuffer.CalcTransform(StencilingSpherePosAndScale, BoundingSphere, View.ViewMatrices.GetPreViewTranslation());
		StencilingGeometryParameters.Set(RHICmdList, this, StencilingSpherePosAndScale);
	}

	void SetParameters(FRHICommandList& RHICmdList, const FViewInfo& View)
	{
		FGlobalShader::SetParameters<FViewUniformShaderParameters>(RHICmdList, RHICmdList.GetBoundVertexShader(),View.ViewUniformBuffer);

		// Don't transform if rendering frustum
		StencilingGeometryParameters.Set(RHICmdList, this, FVector4(0,0,0,1));
	}

private:
	LAYOUT_FIELD(FStencilingGeometryShaderParameters, StencilingGeometryParameters)
	LAYOUT_FIELD(FShaderParameter, ViewId)
};

/**
 * A pixel shader for rendering a texture on a simple element.
 */
class FOcclusionQueryPS : public FGlobalShader
{
	DECLARE_SHADER_TYPE(FOcclusionQueryPS, Global);
public:
	static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters) { return IsFeatureLevelSupported(Parameters.Platform, ERHIFeatureLevel::SM5); }

	FOcclusionQueryPS(const ShaderMetaType::CompiledShaderInitializerType& Initializer) :
		FGlobalShader(Initializer) {}
	FOcclusionQueryPS() {}
};

