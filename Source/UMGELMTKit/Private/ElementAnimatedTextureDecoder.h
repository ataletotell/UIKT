/**
 * Ported from UAnimatedTexture5
 */

#pragma once

#include "CoreMinimal.h"

class FElementAnimatedTextureDecoder
{
public:
	FElementAnimatedTextureDecoder() = default;
	virtual ~FElementAnimatedTextureDecoder() {}

	virtual bool LoadFromMemory(const uint8* InBuffer, uint32 InBufferSize) = 0;
	virtual void Close() = 0;

	/**
	 * @return frame delay in milliseconds
	 */
	virtual uint32 NextFrame(uint32 DefaultFrameDelay, bool bLooping) = 0;
	virtual void SeekToFrame(uint32 FrameIndex) = 0;
	virtual void Reset() = 0;

	virtual uint32 GetWidth() const = 0;
	virtual uint32 GetHeight() const = 0;
	virtual const FColor* GetFrameBuffer() const = 0;

	virtual uint32 GetDuration(uint32 DefaultFrameDelay) const = 0;
	virtual bool SupportsTransparency() const = 0;

public:
	FElementAnimatedTextureDecoder(const FElementAnimatedTextureDecoder&) = delete;
	FElementAnimatedTextureDecoder& operator=(const FElementAnimatedTextureDecoder&) = delete;
};
