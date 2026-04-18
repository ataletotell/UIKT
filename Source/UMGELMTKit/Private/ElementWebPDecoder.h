/**
 * Ported from UAnimatedTexture5
 */

#pragma once

#include "CoreMinimal.h"
#include "ElementAnimatedTextureDecoder.h"
#include "libwebp/src/webp/decode.h"
#include "libwebp/src/webp/demux.h"

/**
* @see https://developers.google.com/speed/webp/docs/api
*/
class FElementWebPDecoder : public FElementAnimatedTextureDecoder
{
public:
	FElementWebPDecoder() = default;
	virtual ~FElementWebPDecoder();

	virtual bool LoadFromMemory(const uint8* InBuffer, uint32 InBufferSize) override;
	virtual void Close() override;

	virtual uint32 NextFrame(uint32 DefaultFrameDelay, bool bLooping) override;
	virtual void SeekToFrame(uint32 FrameIndex) override;
	virtual void Reset() override;

	virtual uint32 GetWidth() const override { return AnimInfo.canvas_width; }
	virtual uint32 GetHeight() const override { return AnimInfo.canvas_height; }
	virtual const FColor* GetFrameBuffer() const override;

	virtual uint32 GetDuration(uint32 DefaultFrameDelay) const override;
	virtual bool SupportsTransparency() const override;

private:
	int PrevFrameTimestamp = 0;
	uint32 Duration = 0;

	WebPAnimInfo AnimInfo;
	WebPBitstreamFeatures Features;

	uint8* FrameBuffer = nullptr;
	WebPAnimDecoder* Decoder = nullptr;
	int mCurrentFrame = 0;
};
