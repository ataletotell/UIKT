/**
 * Ported from UAnimatedTexture5
 */

#include "ElementAnimatedTexture.h"
#include "ElementAnimatedTextureResource.h"
#include "ElementGIFDecoder.h"
#include "ElementWebPDecoder.h"

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
	if (!bPlaying)
		return;
	if (!Decoder)
		return;

	FrameTime += DeltaTime * PlayRate;
	if (FrameTime < FrameDelay)
		return;

	FrameTime = 0;
	FrameDelay = RenderFrameToTexture();
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

float UElementAnimatedTexture::RenderFrameToTexture()
{
	// decode a new frame to memory buffer
	int nFrameDelay = Decoder->NextFrame(DefaultFrameDelay * 1000, bLooping);

	// copy frame to RHI texture
	struct FRenderCommandData
	{
		FTextureResource* RHIResource;
		const uint8* FrameBuffer;
	};

	typedef TSharedPtr<FRenderCommandData, ESPMode::ThreadSafe> FCommandDataPtr;
	FCommandDataPtr CommandData = MakeShared<FRenderCommandData, ESPMode::ThreadSafe>();
	CommandData->RHIResource = GetResource();
	CommandData->FrameBuffer = (const uint8*)(Decoder->GetFrameBuffer());

	//-- equeue render command
	ENQUEUE_RENDER_COMMAND(AnimTexture2D_RenderFrame)(
		[CommandData](FRHICommandListImmediate& RHICmdList)
		{
			if (!CommandData->RHIResource || !CommandData->RHIResource->TextureRHI)
				return;

			FTextureRHIRef Texture2DRHI = CommandData->RHIResource->TextureRHI->GetTexture2D();
			if (!Texture2DRHI)
				return;

			uint32 TexWidth = Texture2DRHI->GetSizeX();
			uint32 TexHeight = Texture2DRHI->GetSizeY();
			uint32 SrcPitch = TexWidth * sizeof(FColor);

			FUpdateTextureRegion2D Region;
			Region.SrcX = Region.SrcY = Region.DestX = Region.DestY = 0;
			Region.Width = TexWidth;
			Region.Height = TexHeight;

			RHIUpdateTexture2D(Texture2DRHI, 0, Region, SrcPitch, CommandData->FrameBuffer);
		});

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
	FrameTime = 0;
	FrameDelay = 0;
	bPlaying = true;
	if (Decoder) Decoder->Reset();
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
	if (Decoder)
	{
		Decoder->SeekToFrame(FrameIndex);
		FrameDelay = RenderFrameToTexture();
		FrameTime = 0;
	}
}
