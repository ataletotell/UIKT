/**
 * Ported from UAnimatedTexture5
 */

#pragma once

#include "CoreMinimal.h"
#include "TextureResource.h"

class UElementAnimatedTexture;

/**
 * FTextureResource implementation for animated 2D textures
 */
class FElementAnimatedTextureResource : public FTextureResource
{
public:
	FElementAnimatedTextureResource(UElementAnimatedTexture* InOwner);

	//~ Begin FTextureResource Interface.
	virtual uint32 GetSizeX() const override;
	virtual uint32 GetSizeY() const override;
	virtual void InitRHI(FRHICommandListBase& RHICmdList) override;
	virtual void ReleaseRHI() override;
	//~ End FTextureResource Interface.

private:
	int32 GetDefaultMipMapBias() const;
	void CreateSamplerStates(float MipMapBias);

private:
	UElementAnimatedTexture* Owner;
};
