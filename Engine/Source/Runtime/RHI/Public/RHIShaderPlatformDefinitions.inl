// Copyright Epic Games, Inc. All Rights Reserved.

/*=============================================================================
	RHIShaderPlatformDefinitions.h: Localizable Friendly Names for Shader Platforms
=============================================================================*/

#pragma once

static FText GetFriendlyShaderPlatformName(const EShaderPlatform InShaderPlatform)
{
	switch (InShaderPlatform)
	{
	case SP_VULKAN_SM5:
	{
		static const FText Description = NSLOCTEXT("FriendlyShaderPlatformNames", "Generic_SM5_loc", "SM5");
		return Description;
	}
	break;

	default:
		if (FStaticShaderPlatformNames::IsStaticPlatform(InShaderPlatform))
		{
			return FDataDrivenShaderPlatformInfo::GetFriendlyName(InShaderPlatform);
		}
		break;
	};

	return FText::GetEmpty();
}
