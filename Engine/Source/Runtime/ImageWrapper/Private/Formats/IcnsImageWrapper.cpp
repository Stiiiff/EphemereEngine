// Copyright Epic Games, Inc. All Rights Reserved.

#include "IcnsImageWrapper.h"


/* FIcnsImageWrapper structors
 *****************************************************************************/

FIcnsImageWrapper::FIcnsImageWrapper()
	: FImageWrapperBase()
{ }


/* FImageWrapper interface
 *****************************************************************************/

bool FIcnsImageWrapper::SetCompressed(const void* InCompressedData, int64 InCompressedSize)
{
	return false;
}


bool FIcnsImageWrapper::SetRaw(const void* InRawData, int64 InRawSize, const int32 InWidth, const int32 InHeight, const ERGBFormat InFormat, const int32 InBitDepth)
{
	return false;
}


void FIcnsImageWrapper::Compress(int32 Quality)
{
	checkf(false, TEXT("ICNS compression not supported"));
}


void FIcnsImageWrapper::Uncompress(const ERGBFormat InFormat, const int32 InBitDepth)
{
	checkf(false, TEXT("ICNS uncompressing not supported on this platform"));
}
