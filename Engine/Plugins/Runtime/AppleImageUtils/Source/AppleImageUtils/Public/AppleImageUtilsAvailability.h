// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#define SUPPORTS_IMAGE_UTILS_1_0 0
#define SUPPORTS_IMAGE_UTILS_2_0 0
#define SUPPORTS_IMAGE_UTILS_2_1 0

class FAppleImageUtilsAvailability
{
public:
	static bool SupportsImageUtils10()
	{
		static bool bSupportsImageUtils10 = false;
#if SUPPORTS_IMAGE_UTILS_1_0
		static bool bSupportChecked = false;
		if (!bSupportChecked)
		{
			bSupportChecked = true;
			// This call is slow, so cache the result
			if (RT_CHECK_IMAGE_UTILS_1_0)
			{
				bSupportsImageUtils10 = true;
			}
		}
#endif
		return bSupportsImageUtils10;
	}

	static bool SupportsImageUtils20()
	{
		static bool bSupportsImageUtils20 = false;
#if SUPPORTS_IMAGE_UTILS_2_0
		static bool bSupportChecked = false;
		if (!bSupportChecked)
		{
			bSupportChecked = true;
			// This call is slow, so cache the result
			if (RT_CHECK_IMAGE_UTILS_2_0)
			{
				bSupportsImageUtils20 = true;
			}
		}
#endif
		return bSupportsImageUtils20;
	}

	static bool SupportsImageUtils21()
	{
		static bool bSupportsImageUtils21 = false;
#if SUPPORTS_IMAGE_UTILS_2_1
		static bool bSupportChecked = false;
		if (!bSupportChecked)
		{
			bSupportChecked = true;
			// This call is slow, so cache the result
			if (RT_CHECK_IMAGE_UTILS_2_1)
			{
				bSupportsImageUtils21 = true;
			}
		}
#endif
		return bSupportsImageUtils21;
	}
};
