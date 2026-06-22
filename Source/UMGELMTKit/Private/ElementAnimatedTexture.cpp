/**
 * Ported from UAnimatedTexture5
 */

#include "ElementAnimatedTexture.h"
#include "ElementAnimatedTextureResource.h"
#include "ElementGIFDecoder.h"
#include "ElementWebPDecoder.h"
#include "Async/Async.h"

float UElementAnimatedTexture::GetSurfaceWidth() const
{
	if (Decoder) return Decoder->GetWidth();
	return 1.0f;
}

float UElementAnimatedTexture::GetSurfaceHeight() const
{
	if (Decoder) return Decoder->GetHeight();
	return 1.0f;
}

FTextureResource* UElementAnimatedTexture::CreateResource()
{
	if (FileType == EElementAnimatedTextureType::None
		|| FileBlob.Num() <= 0)
		return nullptr;

	// create decoder
	switch (FileType)
	{
	case EElementAnimatedTextureType::Gif:
		Decoder = MakeShared<FElementGIFDecoder, ESPMode::ThreadSafe>();
		break;
	case EElementAnimatedTextureType::Webp:
		Decoder = MakeShared<FElementWebPDecoder, ESPMode::ThreadSafe>();
		break;
	}

	check(Decoder);
	if (Decoder->LoadFromMemory(FileBlob.GetData(), FileBlob.Num()))
	{
		AnimationLength = Decoder->GetDuration(DefaultFrameDelay * 1000) / 1000.0f;
		SupportsTransparency = Decoder->SupportsTransparency();
	}
	else
	{
		Decoder.Reset();
		return nullptr;
	}

	// create RHI resource object
	FTextureResource* NewResource = new FElementAnimatedTextureResource(this);
	return NewResource;
}

void UElementAnimatedTexture::Tick(float DeltaTime)
{
	if (!bPlaying || !Decoder)
		return;

	FrameTime += DeltaTime * PlayRate;
	if (FrameTime < FrameDelay || bDecodeTaskPending)
		return;

	bDecodeTaskPending = true;
	FrameTime = 0;

	TSharedPtr<FElementAnimatedTextureDecoder, ESPMode::ThreadSafe> LocalDecoder = Decoder;
	TSharedPtr<FCriticalSection> LocalLock = DecoderLock;
	FTextureResource* RHIResource = GetResource();
	const uint32 DefaultDelayMs = static_cast<uint32>(DefaultFrameDelay * 1000.f);
	const bool bLoop = bLooping;
	const uint32 Generation = DecodeGeneration;
	TWeakObjectPtr<UElementAnimatedTexture> WeakThis(this);

	Async(EAsyncExecution::ThreadPool,
		[WeakThis, LocalDecoder, LocalLock, RHIResource, DefaultDelayMs, bLoop, Generation]()
		{
			int32 DelayMs;
			const uint8* FrameBuffer;
			{
				FScopeLock Lock(LocalLock.Get());
				DelayMs = static_cast<int32>(LocalDecoder->NextFrame(DefaultDelayMs, bLoop));
				FrameBuffer = reinterpret_cast<const uint8*>(LocalDecoder->GetFrameBuffer());
			}

			ENQUEUE_RENDER_COMMAND(AnimTexture2D_RenderFrame)(
				[WeakThis, RHIResource, FrameBuffer, DelayMs, Generation](FRHICommandListImmediate& RHICmdList)
				{
					if (RHIResource && RHIResource->TextureRHI)
					{
						FTextureRHIRef Texture2DRHI = RHIResource->TextureRHI->GetTexture2D();
						if (Texture2DRHI)
						{
							uint32 TexWidth = Texture2DRHI->GetSizeX();
							uint32 TexHeight = Texture2DRHI->GetSizeY();
							FUpdateTextureRegion2D Region;
							Region.SrcX = Region.SrcY = Region.DestX = Region.DestY = 0;
							Region.Width = TexWidth;
							Region.Height = TexHeight;
							RHIUpdateTexture2D(Texture2DRHI, 0, Region, TexWidth * sizeof(FColor), FrameBuffer);
						}
					}

					Async(EAsyncExecution::TaskGraphMainThread,
						[WeakThis, DelayMs, Generation]()
						{
							if (UElementAnimatedTexture* Self = WeakThis.Get())
							{
								if (Self->DecodeGeneration == Generation)
								{
									Self->FrameDelay = static_cast<float>(DelayMs) / 1000.f;
									Self->bDecodeTaskPending = false;
								}
							}
						});
				});
		});
}


#if WITH_EDITOR
void UElementAnimatedTexture::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	bool RequiresNotifyMaterials = false;
	bool ResetAnimState = false;

	FProperty* PropertyThatChanged = PropertyChangedEvent.Property;
	if (PropertyThatChanged)
	{
		const FName PropertyName = PropertyThatChanged->GetFName();

		static const FName SupportsTransparencyName = GET_MEMBER_NAME_CHECKED(UElementAnimatedTexture, SupportsTransparency);

		if (PropertyName == SupportsTransparencyName)
		{
			RequiresNotifyMaterials = true;
			ResetAnimState = true;
		}
	}// end of if(prop is valid)

	if (ResetAnimState)
	{
		FrameDelay = RenderFrameToTexture();
		FrameTime = 0;
	}

	if (RequiresNotifyMaterials)
		NotifyMaterials();
}
#endif // WITH_EDITOR

void UElementAnimatedTexture::ImportFile(EElementAnimatedTextureType InFileType, const uint8* InBuffer, uint32 InBufferSize)
{
	FileType = InFileType;
	FileBlob = TArray<uint8>(InBuffer, InBufferSize);
}

void UElementAnimatedTexture::UploadFrameToRHI(const uint8* FrameBuffer)
{
	FTextureResource* RHIResource = GetResource();
	ENQUEUE_RENDER_COMMAND(AnimTexture2D_RenderFrame)(
		[RHIResource, FrameBuffer](FRHICommandListImmediate& RHICmdList)
		{
			if (!RHIResource || !RHIResource->TextureRHI)
				return;
			FTextureRHIRef Texture2DRHI = RHIResource->TextureRHI->GetTexture2D();
			if (!Texture2DRHI)
				return;
			uint32 TexWidth = Texture2DRHI->GetSizeX();
			uint32 TexHeight = Texture2DRHI->GetSizeY();
			FUpdateTextureRegion2D Region;
			Region.SrcX = Region.SrcY = Region.DestX = Region.DestY = 0;
			Region.Width = TexWidth;
			Region.Height = TexHeight;
			RHIUpdateTexture2D(Texture2DRHI, 0, Region, TexWidth * sizeof(FColor), FrameBuffer);
		});
}

float UElementAnimatedTexture::RenderFrameToTexture()
{
	int32 nFrameDelay;
	const uint8* FrameBuffer;
	{
		FScopeLock Lock(DecoderLock.Get());
		nFrameDelay = static_cast<int32>(Decoder->NextFrame(static_cast<uint32>(DefaultFrameDelay * 1000), bLooping));
		FrameBuffer = reinterpret_cast<const uint8*>(Decoder->GetFrameBuffer());
	}
	UploadFrameToRHI(FrameBuffer);
	return nFrameDelay / 1000.0f;
}

float UElementAnimatedTexture::GetAnimationLength() const
{
	return AnimationLength;
}

void UElementAnimatedTexture::Play()
{
	bPlaying = true;
}

void UElementAnimatedTexture::PlayFromStart()
{
	++DecodeGeneration;
	bDecodeTaskPending = false;
	FrameTime = 0;
	FrameDelay = 0;
	bPlaying = true;
	if (Decoder)
	{
		int32 nFrameDelay;
		const uint8* FrameBuffer;
		{
			FScopeLock Lock(DecoderLock.Get());
			Decoder->Reset();
			nFrameDelay = static_cast<int32>(Decoder->NextFrame(static_cast<uint32>(DefaultFrameDelay * 1000), bLooping));
			FrameBuffer = reinterpret_cast<const uint8*>(Decoder->GetFrameBuffer());
		}
		FrameDelay = nFrameDelay / 1000.f;
		UploadFrameToRHI(FrameBuffer);
	}
}

void UElementAnimatedTexture::Stop()
{
	bPlaying = false;
}

void UElementAnimatedTexture::PlayFromFrame(int32 FrameIndex)
{
	SetCurrentFrame(FrameIndex);
	Play();
}

void UElementAnimatedTexture::SetCurrentFrame(int32 FrameIndex)
{
	if (!Decoder)
		return;

	++DecodeGeneration;
	bDecodeTaskPending = false;

	int32 nFrameDelay;
	const uint8* FrameBuffer;
	{
		FScopeLock Lock(DecoderLock.Get());
		Decoder->SeekToFrame(FrameIndex);
		nFrameDelay = static_cast<int32>(Decoder->NextFrame(static_cast<uint32>(DefaultFrameDelay * 1000), bLooping));
		FrameBuffer = reinterpret_cast<const uint8*>(Decoder->GetFrameBuffer());
	}
	FrameDelay = nFrameDelay / 1000.f;
	FrameTime = 0;
	UploadFrameToRHI(FrameBuffer);
}
