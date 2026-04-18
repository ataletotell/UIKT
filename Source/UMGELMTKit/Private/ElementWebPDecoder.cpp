/**
 * Ported from UAnimatedTexture5
 */

#include "ElementWebPDecoder.h"

// Define a log category (static to this file)
DEFINE_LOG_CATEGORY_STATIC(LogElementAnimTexture, Log, All);

FElementWebPDecoder::~FElementWebPDecoder()
{
	Close();
}

bool FElementWebPDecoder::LoadFromMemory(const uint8* InBuffer, uint32 InBufferSize)
{
	// get image width height
	int ret = WebPGetFeatures(InBuffer, InBufferSize, &Features);
	if (ret != VP8_STATUS_OK)
	{
		UE_LOG(LogElementAnimTexture, Error, TEXT("FElementWebPDecoder: Get Webp info failed, %d."), ret);
		return false;
	}

	// create WebPAnimDecoder
	WebPAnimDecoderOptions opt;
	WebPAnimDecoderOptionsInit(&opt);
	opt.color_mode = MODE_BGRA;
	opt.use_threads = 0;

	WebPData WData = { InBuffer, InBufferSize };
	Decoder = WebPAnimDecoderNew(&WData, &opt);
	if (!Decoder)
	{
		UE_LOG(LogElementAnimTexture, Error, TEXT("FElementWebPDecoder: Error parsing image."));
		return false;
	}

	// get anim info
	if (!WebPAnimDecoderGetInfo(Decoder, &AnimInfo)) {
		UE_LOG(LogElementAnimTexture, Error, TEXT("FElementWebPDecoder: Error getting global info about the animation."));
		return false;
	}

	// TODO: Optimization - calculate duration without full decoding if possible
	int Timestamp = 0;
	uint8* Buffer = nullptr;
	while (WebPAnimDecoderHasMoreFrames(Decoder)) {
		WebPAnimDecoderGetNext(Decoder, &Buffer, &Timestamp);
	}
	WebPAnimDecoderReset(Decoder);
	Duration = Timestamp;

	return true;
}

void FElementWebPDecoder::Close()
{
	if (Decoder)
	{
		WebPAnimDecoderDelete(Decoder);
		Decoder = nullptr;
		FrameBuffer = nullptr;
	}
}

uint32 FElementWebPDecoder::NextFrame(uint32 DefaultFrameDelay, bool bLooping)
{
	if (Decoder == nullptr)
		return DefaultFrameDelay;

	// restart
	if (!WebPAnimDecoderHasMoreFrames(Decoder))
	{
		if (bLooping)
			WebPAnimDecoderReset(Decoder);
		else
			return DefaultFrameDelay;
	}

	// decode next frame
	int Timestamp = 0;
	if (!WebPAnimDecoderGetNext(Decoder, &FrameBuffer, &Timestamp))
	{
		UE_LOG(LogElementAnimTexture, Error, TEXT("FElementWebPDecoder: Error decoding frame."));
	}
	
	mCurrentFrame++;

	// frame duration
	int FrameDuration = Timestamp - PrevFrameTimestamp;
	PrevFrameTimestamp = Timestamp;
	return FrameDuration;
}

void FElementWebPDecoder::SeekToFrame(uint32 FrameIndex)
{
	if (!Decoder) return;

	uint32 FrameCount = AnimInfo.frame_count;
	if (FrameIndex >= FrameCount)
	{
		FrameIndex = 0;
	}

	if (FrameIndex < (uint32)mCurrentFrame)
	{
		Reset();
	}

	while ((uint32)mCurrentFrame < FrameIndex)
	{
		NextFrame(0, false);
	}
}

void FElementWebPDecoder::Reset()
{
	if (Decoder)
		WebPAnimDecoderReset(Decoder);
	FrameBuffer = nullptr;
	mCurrentFrame = 0;
	PrevFrameTimestamp = 0;
}

const FColor* FElementWebPDecoder::GetFrameBuffer() const
{
	return (FColor*)FrameBuffer;
}

uint32 FElementWebPDecoder::GetDuration(uint32 DefaultFrameDelay) const
{
	return Duration;
}

bool FElementWebPDecoder::SupportsTransparency() const
{
	return Features.has_alpha != 0;
}
