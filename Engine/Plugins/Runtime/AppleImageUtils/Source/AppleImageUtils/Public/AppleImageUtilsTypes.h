// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "UObject/Interface.h"
#include "AppleImageUtilsTypes.generated.h"

UENUM(BlueprintType, Category="AppleImageUtils", meta=(Experimental))
enum class ETextureRotationDirection : uint8
{
	None,
	Left,
	Right,
	Down,
	LeftMirrored,
	RightMirrored,
	DownMirrored,
	UpMirrored
};

UENUM(BlueprintType, Category="AppleImageUtils", meta=(Experimental))
enum class EAppleTextureType : uint8
{
	Unknown,
	Image,
	PixelBuffer,
	Surface,
	MetalTexture
};

/** Dummy class needed to support Cast<IAppleImageInterface>(Object). */
UINTERFACE()
class APPLEIMAGEUTILS_API UAppleImageInterface : public UInterface
{
	GENERATED_UINTERFACE_BODY()
};

/**
 * Base class for accessing the raw Apple image data
 */
class APPLEIMAGEUTILS_API IAppleImageInterface
{
	GENERATED_IINTERFACE_BODY()

public:
	/** @return the type of image held by the implementing object */
	virtual EAppleTextureType GetTextureType() const = 0;
};
