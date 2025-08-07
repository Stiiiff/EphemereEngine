// Copyright Epic Games, Inc. All Rights Reserved.

/*=============================================================================
	RHIShaderFormatDefinitions.h: Names for Shader Formats
		(that don't require linking).
=============================================================================*/

#pragma once

static FName NAME_VULKAN_SM5(TEXT("SF_VULKAN_SM5"));

static FName ShaderPlatformToShaderFormatName(EShaderPlatform Platform)
{
	return NAME_VULKAN_SM5;
	/*switch (Platform)
	{
	case SP_VULKAN_SM5:
		return NAME_VULKAN_SM5;

	default:
		if (FStaticShaderPlatformNames::IsStaticPlatform(Platform))
		{
			return FStaticShaderPlatformNames::Get().GetShaderFormat(Platform);
		}
		else
		{
			checkf(0, TEXT("Unknown EShaderPlatform %d!"), (int32)Platform);
			return NAME_VULKAN_SM5;
		}
	}*/
}

static EShaderPlatform ShaderFormatNameToShaderPlatform(FName ShaderFormat)
{
	return SP_VULKAN_SM5;
	/*if (ShaderFormat == NAME_VULKAN_SM5)				return SP_VULKAN_SM5;

	for (int32 StaticPlatform = SP_StaticPlatform_First; StaticPlatform <= SP_StaticPlatform_Last; ++StaticPlatform)
	{
		if (ShaderFormat == FStaticShaderPlatformNames::Get().GetShaderFormat(EShaderPlatform(StaticPlatform)))
		{
			return EShaderPlatform(StaticPlatform);
		}
	}

	return SP_NumPlatforms;*/
}
