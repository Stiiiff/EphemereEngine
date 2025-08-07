// Copyright Epic Games, Inc. All Rights Reserved.

#include "VideoEncoderInput.h"
#include "VideoEncoderInputImpl.h"
#include "VideoEncoderCommon.h"
#include "VideoEncoderFactory.h"
#include "AVEncoderDebug.h"
#include "Misc/Paths.h"
#include "GenericPlatform/GenericPlatformMath.h"

#include "Misc/Guid.h"

#if PLATFORM_DESKTOP
#include "VulkanRHIBridge.h"
#endif

#if PLATFORM_WINDOWS
#include "MicrosoftCommon.h"
#endif

FString GetGUID()
{
	static FGuid id;
	if (!id.IsValid())
	{
		id = FGuid::NewGuid();
	}

	return id.ToString();
}

namespace AVEncoder
{

// *** FVideoEncoderInput *************************************************************************

// --- construct video encoder input based on expected input frame format -------------------------

TSharedPtr<FVideoEncoderInput> FVideoEncoderInput::CreateDummy(uint32 InWidth, uint32 InHeight, bool bIsResizable)
{
	TSharedPtr<FVideoEncoderInputImpl>	Input = MakeShared<FVideoEncoderInputImpl>();
	Input->bIsResizable = bIsResizable;

	if (!Input->SetupForDummy(InWidth, InHeight))
	{
		Input.Reset();
	}
	return Input;
}

TSharedPtr<FVideoEncoderInput> FVideoEncoderInput::CreateForYUV420P(uint32 InWidth, uint32 InHeight, bool bIsResizable)
{
	TSharedPtr<FVideoEncoderInputImpl>	Input = MakeShared<FVideoEncoderInputImpl>();
	Input->bIsResizable = bIsResizable;

	if (!Input->SetupForYUV420P(InWidth, InHeight))
	{
		Input.Reset();
	}
	return Input;
}

TSharedPtr<FVideoEncoderInput> FVideoEncoderInput::CreateForCUDA(void* InApplicationContext, uint32 InWidth, uint32 InHeight, bool bIsResizable)
{
	TSharedPtr<FVideoEncoderInputImpl> Input = MakeShared<FVideoEncoderInputImpl>();
	Input->bIsResizable = bIsResizable;
		
	if (!Input->SetupForCUDA(reinterpret_cast<CUcontext>(InApplicationContext), InWidth, InHeight))
	{
		Input.Reset();
	}
	return Input;
}

#if PLATFORM_DESKTOP
TSharedPtr<FVideoEncoderInput> FVideoEncoderInput::CreateForVulkan(void* InApplicationVulkanData, uint32 InWidth, uint32 InHeight, bool bIsResizable)
{
	TSharedPtr<FVideoEncoderInputImpl>	Input = MakeShared<FVideoEncoderInputImpl>();
	Input->bIsResizable = bIsResizable;

	FVulkanDataStruct* VulkanData = static_cast<FVulkanDataStruct*>(InApplicationVulkanData);

	if (!Input->SetupForVulkan(	VulkanData->VulkanInstance, VulkanData->VulkanPhysicalDevice, VulkanData->VulkanDevice, InWidth, InHeight))
	{
		Input.Reset();
	}

	return Input;
}
#endif

void FVideoEncoderInput::SetResolution(uint32 InWidth, uint32 InHeight)
{
	Width = InWidth;
	Height = InHeight;
}

void FVideoEncoderInput::SetMaxNumBuffers(uint32 InMaxNumBuffers)
{
	MaxNumBuffers = InMaxNumBuffers;
}

// --- encoder input frames -----------------------------------------------------------------------



// *** FVideoEncoderInputImpl *********************************************************************

FVideoEncoderInputImpl::~FVideoEncoderInputImpl()
{
	{
		FScopeLock						Guard(&ProtectFrames);
		if (ActiveFrames.Num() > 0)
		{
			UE_LOG(LogVideoEncoder, Error, TEXT("There are still %d active input frames."), ActiveFrames.Num());
		}
		
		check(ActiveFrames.Num() == 0);
		
		while (!AvailableFrames.IsEmpty())
		{
			FVideoEncoderInputFrameImpl* Frame = nullptr;
			AvailableFrames.Dequeue(Frame);
			delete Frame;
		}
	}
}

bool FVideoEncoderInputImpl::SetupForDummy(uint32 InWidth, uint32 InHeight)
{
	FrameFormat = EVideoFrameFormat::Undefined;
	this->Width = InWidth;
	this->Height = InHeight;
	return true;
}

bool FVideoEncoderInputImpl::SetupForYUV420P(uint32 InWidth, uint32 InHeight)
{
	FrameFormat = EVideoFrameFormat::YUV420P;
	this->Width = InWidth;
	this->Height = InHeight;
	FrameInfoYUV420P.StrideY = InWidth;
	FrameInfoYUV420P.StrideU = (InWidth + 1) / 2;
	FrameInfoYUV420P.StrideV = (InWidth + 1) / 2;

	CollectAvailableEncoders();
	return true;
}

bool FVideoEncoderInputImpl::SetupForCUDA(void* InApplicationContext, uint32 InWidth, uint32 InHeight)
{
	FrameInfoCUDA.EncoderContextCUDA = static_cast<CUcontext>(InApplicationContext);

	FrameFormat = EVideoFrameFormat::CUDA_R8G8B8A8_UNORM;
	this->Width = InWidth;
	this->Height = InHeight;

	CollectAvailableEncoders();
	return true;
}

#if PLATFORM_DESKTOP
bool FVideoEncoderInputImpl::SetupForVulkan(VkInstance InApplicationVulkanInstance, VkPhysicalDevice InApplicationVulkanPhysicalDevice, VkDevice InApplicationVulkanDevice, uint32 InWidth, uint32 InHeight)
{
	FrameInfoVulkan.VulkanInstance = InApplicationVulkanInstance;
	FrameInfoVulkan.VulkanPhysicalDevice = InApplicationVulkanPhysicalDevice;
	FrameInfoVulkan.VulkanDevice = InApplicationVulkanDevice;

	FrameFormat = EVideoFrameFormat::VULKAN_R8G8B8A8_UNORM;
	this->Width = InWidth;
	this->Height = InHeight;

	CollectAvailableEncoders();
	return true;
}
#endif

// --- available encoders -------------------------------------------------------------------------

void FVideoEncoderInputImpl::CollectAvailableEncoders()
{
	AvailableEncoders.Empty();
	for (const FVideoEncoderInfo& Info : FVideoEncoderFactory::Get().GetAvailable())
	{
		if (Info.SupportedInputFormats.Contains(FrameFormat))
		{
			AvailableEncoders.Push(Info);
		}
	}
}

const TArray<FVideoEncoderInfo>& FVideoEncoderInputImpl::GetAvailableEncoders()
{
	return AvailableEncoders;
}

// --- encoder input frames -----------------------------------------------------------------------

// create a user managed buffer
FVideoEncoderInputFrame* FVideoEncoderInputImpl::CreateBuffer(OnFrameReleasedCallback InOnFrameReleased)
{
	FVideoEncoderInputFrameImpl* Frame = CreateFrame();
	if (Frame)
	{
		FScopeLock						Guard(&ProtectFrames);
		UserManagedFrames.Emplace(Frame, MoveTemp(InOnFrameReleased));
	}
	return Frame;
}

// destroy user managed buffer
void FVideoEncoderInputImpl::DestroyBuffer(FVideoEncoderInputFrame* InBuffer)
{
	FVideoEncoderInputFrameImpl*	Frame = static_cast<FVideoEncoderInputFrameImpl*>(InBuffer);
	FScopeLock						Guard(&ProtectFrames);
	bool							bAnythingRemoved = false;
	for (int32 Index = UserManagedFrames.Num() - 1; Index >= 0; --Index)
	{
		if (UserManagedFrames[Index].Key == Frame)
		{
			UserManagedFrames.RemoveAt(Index);
			bAnythingRemoved = true;
		}
	}
	if (bAnythingRemoved)
	{
		delete Frame;
	}
}

// --- encoder input frames -----------------------------------------------------------------------

FVideoEncoderInputFrame* FVideoEncoderInputImpl::ObtainInputFrame()
{
	FVideoEncoderInputFrameImpl*	Frame = nullptr;
	FScopeLock						Guard(&ProtectFrames);

	if (!AvailableFrames.IsEmpty())
	{

		AvailableFrames.Dequeue(Frame);
		
	}
	else 
	{
		Frame = CreateFrame();
		UE_LOG(LogVideoEncoder, Verbose, TEXT("Created new frame total frames: %d"), NumBuffers);
	}

	ActiveFrames.Push(Frame);
	
	Frame->SetFrameID(NextFrameID++);

	if (NextFrameID == 0)
	{
		++NextFrameID; // skip 0 id
	} 

	return const_cast<FVideoEncoderInputFrame*>(Frame->Obtain());
}

FVideoEncoderInputFrameImpl* FVideoEncoderInputImpl::CreateFrame()
{
	FVideoEncoderInputFrameImpl*	Frame = new FVideoEncoderInputFrameImpl(this);
	NumBuffers++;
	switch (FrameFormat)
	{
	case EVideoFrameFormat::Undefined:
		UE_LOG(LogVideoEncoder, Error, TEXT("Got undefined frame format!"))
		break;
	case EVideoFrameFormat::YUV420P:
		SetupFrameYUV420P(Frame);
		break;
	case EVideoFrameFormat::D3D11_R8G8B8A8_UNORM:
		break;
	case EVideoFrameFormat::D3D12_R8G8B8A8_UNORM:
		break;
	case EVideoFrameFormat::VULKAN_R8G8B8A8_UNORM:
		SetupFrameVulkan(Frame);
		break;
	case EVideoFrameFormat::CUDA_R8G8B8A8_UNORM:
		SetupFrameCUDA(Frame);
		break;
	default:
		check(false);
		break;
	}
	return Frame;
}

void FVideoEncoderInputImpl::ReleaseInputFrame(FVideoEncoderInputFrame* InFrame)
{
	FVideoEncoderInputFrameImpl* InFrameImpl = static_cast<FVideoEncoderInputFrameImpl*>(InFrame);

	FScopeLock	Guard(&ProtectFrames);
	// check user managed buffers first
	for (const UserManagedFrame& Frame : UserManagedFrames)
	{
		if (Frame.Key == InFrameImpl)
		{
			Frame.Value(InFrameImpl);
			return;
		}
	}

	int32		NumRemoved = ActiveFrames.Remove(InFrameImpl);
	check(NumRemoved == 1);
	if (NumRemoved > 0)
	{
		// drop frame if format changed
		if (InFrame->GetFormat() != FrameFormat)
		{
			ProtectFrames.Unlock();
			delete InFrameImpl;
			NumBuffers--;
			UE_LOG(LogVideoEncoder, Verbose, TEXT("Deleted buffer (format mismatch) total remaining: %d"), NumBuffers);
			return;
		}
		
		// drop frame if resolution changed
		if(bIsResizable && (InFrame->GetWidth() != this->Width || InFrame->GetHeight() != this->Height))
		{
			ProtectFrames.Unlock();
			delete InFrameImpl;
			NumBuffers--;
			UE_LOG(LogVideoEncoder, Verbose, TEXT("Deleted buffer (size mismatch) total remaining: %d"), NumBuffers);
			return;
		}

		if(!AvailableFrames.IsEmpty() && NumBuffers > MaxNumBuffers)
		{
			ProtectFrames.Unlock();
			delete InFrameImpl;
			NumBuffers--;
			UE_LOG(LogVideoEncoder, Verbose, TEXT("Deleted buffer (too many) total frames: %d"), NumBuffers);
			return;
		}

		AvailableFrames.Enqueue(InFrameImpl);
	}
}

void FVideoEncoderInputImpl::Flush()
{
	ProtectFrames.Lock();
	while (!AvailableFrames.IsEmpty())
	{
		FVideoEncoderInputFrameImpl*	Frame = nullptr;
		AvailableFrames.Dequeue(Frame);
		ProtectFrames.Unlock();
		delete Frame;
		NumBuffers--;
		ProtectFrames.Lock();
	}
	ProtectFrames.Unlock();
}

void FVideoEncoderInputImpl::SetupFrameYUV420P(FVideoEncoderInputFrameImpl* Frame)
{
	Frame->SetFormat(EVideoFrameFormat::YUV420P);
	Frame->SetWidth(this->Width);
	Frame->SetHeight(this->Height);
	FVideoEncoderInputFrame::FYUV420P& YUV420P = Frame->GetYUV420P();
	YUV420P.StrideY = FrameInfoYUV420P.StrideY;
	YUV420P.StrideU = FrameInfoYUV420P.StrideU;
	YUV420P.StrideV = FrameInfoYUV420P.StrideV;
	YUV420P.Data[0] = YUV420P.Data[1] = YUV420P.Data[2] = nullptr;
}

void FVideoEncoderInputImpl::SetupFrameVulkan(FVideoEncoderInputFrameImpl* Frame)
{
#if PLATFORM_DESKTOP
	Frame->SetFormat(FrameFormat);
	Frame->SetWidth(this->Width);
	Frame->SetHeight(this->Height);

	FVideoEncoderInputFrame::FVulkan& Data = Frame->GetVulkan();
	Data.EncoderDevice = FrameInfoVulkan.VulkanDevice;
#endif
}

void FVideoEncoderInputImpl::SetupFrameCUDA(FVideoEncoderInputFrameImpl* Frame)
{
	Frame->SetFormat(FrameFormat);
	Frame->SetWidth(this->Width);
	Frame->SetHeight(this->Height);
	FVideoEncoderInputFrame::FCUDA& Data = Frame->GetCUDA();
	Data.EncoderDevice = FrameInfoCUDA.EncoderContextCUDA;
}

// ---

CUcontext FVideoEncoderInputImpl::GetCUDAEncoderContext() const
{
	return FrameInfoCUDA.EncoderContextCUDA;
}

#if PLATFORM_DESKTOP
void* FVideoEncoderInputImpl::GetVulkanEncoderDevice() const
{
	return (void*)&FrameInfoVulkan;
}
#endif

// *** FVideoEncoderInputFrame ********************************************************************

FVideoEncoderInputFrame::FVideoEncoderInputFrame()
	: FrameID(0)
	, TimestampUs(0)
	, TimestampRTP(0)
	, NumReferences(0)
	, Format(EVideoFrameFormat::Undefined)
	, Width(0)
	, Height(0)
	, bFreeYUV420PData(false)
{
}

FVideoEncoderInputFrame::FVideoEncoderInputFrame(const FVideoEncoderInputFrame& CloneFrom)
	: FrameID(CloneFrom.FrameID)
	, TimestampUs(CloneFrom.TimestampUs)
	, TimestampRTP(CloneFrom.TimestampRTP)
	, NumReferences(0)
	, Format(CloneFrom.Format)
	, Width(CloneFrom.Width)
	, Height(CloneFrom.Height)
	, bFreeYUV420PData(false)
{

	CUDA.EncoderDevice = CloneFrom.CUDA.EncoderDevice;
	CUDA.EncoderTexture = CloneFrom.CUDA.EncoderTexture;
}

FVideoEncoderInputFrame::~FVideoEncoderInputFrame()
{
	if (bFreeYUV420PData)
	{
		delete[] YUV420P.Data[0];
		delete[] YUV420P.Data[1];
		delete[] YUV420P.Data[2];
		YUV420P.Data[0] = YUV420P.Data[1] = YUV420P.Data[2] = nullptr;
		bFreeYUV420PData = false;
	}

	if (CUDA.EncoderTexture)
	{
		OnReleaseCUDATexture(CUDA.EncoderTexture);
		CUDA.EncoderTexture = nullptr;
	}

#if PLATFORM_DESKTOP
	if (Vulkan.EncoderTexture != VK_NULL_HANDLE)
	{
		OnReleaseVulkanTexture(Vulkan.EncoderTexture);
	}

	if (Vulkan.EncoderSurface)
	{
		OnReleaseVulkanSurface(Vulkan.EncoderSurface);
	}
#endif
}

void FVideoEncoderInputFrame::AllocateYUV420P()
{
	if (!bFreeYUV420PData)
	{
		YUV420P.StrideY = Width;
		YUV420P.StrideU = (Width + 1) / 2;
		YUV420P.StrideV = (Width + 1) / 2;;
		YUV420P.Data[0] = new uint8[Height * YUV420P.StrideY];
		YUV420P.Data[1] = new uint8[(Height + 1) / 2 * YUV420P.StrideU];
		YUV420P.Data[2] = new uint8[(Height + 1) / 2 * YUV420P.StrideV];
		bFreeYUV420PData = true;
	}
}

void FVideoEncoderInputFrame::SetYUV420P(const uint8* InDataY, const uint8* InDataU, const uint8* InDataV, uint32 InStrideY, uint32 InStrideU, uint32 InStrideV)
{
	if (Format == EVideoFrameFormat::YUV420P)
	{
		if (bFreeYUV420PData)
		{
			delete[] YUV420P.Data[0];
			delete[] YUV420P.Data[1];
			delete[] YUV420P.Data[2];
			bFreeYUV420PData = false;
		}
		YUV420P.Data[0] = InDataY;
		YUV420P.Data[1] = InDataU;
		YUV420P.Data[2] = InDataV;
		YUV420P.StrideY = InStrideY;
		YUV420P.StrideU = InStrideU;
		YUV420P.StrideV = InStrideV;
	}
}

static FThreadSafeCounter	_VideoEncoderInputFrameCnt{ 0 };

void FVideoEncoderInputFrame::SetTexture(CUarray InTexture, FReleaseCUDATextureCallback InOnReleaseTexture)
{
	if (Format == EVideoFrameFormat::CUDA_R8G8B8A8_UNORM)
	{
		CUDA.EncoderTexture = InTexture;
		OnReleaseCUDATexture = InOnReleaseTexture;
		if (!CUDA.EncoderTexture)
		{
			UE_LOG(LogVideoEncoder, Warning, TEXT("SetTexture | Cuda device pointer is null"));
		}
	}
}


#if PLATFORM_DESKTOP
void FVideoEncoderInputFrame::SetTexture(VkImage InTexture, FReleaseVulkanTextureCallback InOnReleaseTexture)
{
	if (Format == EVideoFrameFormat::VULKAN_R8G8B8A8_UNORM)
	{
		Vulkan.EncoderTexture = InTexture;		
		OnReleaseVulkanTexture = InOnReleaseTexture;
		if (!Vulkan.EncoderTexture)
		{
			UE_LOG(LogVideoEncoder, Warning, TEXT("SetTexture | Vulkan VkImage is null"));
		}
	}
}

void FVideoEncoderInputFrame::SetTexture(VkImage InTexture, VkDeviceMemory InTextureDeviceMemory, uint64 InTextureMemorySize, FReleaseVulkanTextureCallback InOnReleaseTexture)
{
	if (Format == EVideoFrameFormat::VULKAN_R8G8B8A8_UNORM)
	{
		Vulkan.EncoderTexture = InTexture;
		Vulkan.EncoderDeviceMemory = InTextureDeviceMemory;
		Vulkan.EncoderMemorySize = InTextureMemorySize;
		OnReleaseVulkanTexture = InOnReleaseTexture;

		if (!Vulkan.EncoderTexture || !InTextureDeviceMemory)
		{
			UE_LOG(LogVideoEncoder, Warning, TEXT("SetTexture | Vulkan VkImage or VkTextureDeviceMemory is null"));
		}
	}
}
#endif

// *** FVideoEncoderInputFrameImpl ****************************************************************

FVideoEncoderInputFrameImpl::FVideoEncoderInputFrameImpl(FVideoEncoderInputImpl* InInput)
	: Input(InInput)
{
}

FVideoEncoderInputFrameImpl::FVideoEncoderInputFrameImpl(const FVideoEncoderInputFrameImpl& InCloneFrom, FCloneDestroyedCallback InCloneDestroyedCallback)
	: FVideoEncoderInputFrame(InCloneFrom)
	, Input(InCloneFrom.Input)
	, ClonedReference(InCloneFrom.Obtain())
	, OnCloneDestroyed(MoveTemp(InCloneDestroyedCallback))
{
}

FVideoEncoderInputFrameImpl::~FVideoEncoderInputFrameImpl()
{
	if (ClonedReference)
	{
		ClonedReference->Release();
	}
	else
	{
	}
}

void FVideoEncoderInputFrameImpl::Release() const
{
	if (NumReferences.Decrement() == 0)
	{
		if (ClonedReference)
		{
			OnCloneDestroyed(this);
			delete this;
		}
		else
		{
			Input->ReleaseInputFrame(const_cast<FVideoEncoderInputFrameImpl*>(this));
		}
	}
}

// Clone frame - this will create a copy that references the original until destroyed
const FVideoEncoderInputFrame* FVideoEncoderInputFrameImpl::Clone(FCloneDestroyedCallback InCloneDestroyedCallback) const
{
	FVideoEncoderInputFrameImpl*	ClonedFrame = new FVideoEncoderInputFrameImpl(*this, MoveTemp(InCloneDestroyedCallback));
	return ClonedFrame;
}



} /* namespace AVEncoder */
