/**
 * Ported from UAnimatedTexture5
 */

#include "ElementAnimatedTextureThumbnailRenderer.h"
#include "ElementAnimatedTexture.h"

#include "CanvasTypes.h"
#include "CanvasItem.h"
#include "Engine/Texture2D.h"
#include "ThumbnailRendering/ThumbnailManager.h"

void UElementAnimatedTextureThumbnailRenderer::GetThumbnailSize(UObject* Object, float Zoom, uint32& OutWidth, uint32& OutHeight) const
{
	UElementAnimatedTexture* Texture = Cast<UElementAnimatedTexture>(Object);

	if (Texture != nullptr)
	{
		OutWidth = FMath::TruncToInt(Zoom * (float)Texture->GetSurfaceWidth());
		OutHeight = FMath::TruncToInt(Zoom * (float)Texture->GetSurfaceHeight());
	}
	else
	{
		OutWidth = OutHeight = 0;
	}
}

void UElementAnimatedTextureThumbnailRenderer::Draw(UObject* Object, int32 X, int32 Y, uint32 Width, uint32 Height, FRenderTarget* Viewport, FCanvas* Canvas, bool bAdditionalViewFamily)
{
	UElementAnimatedTexture* Texture = Cast<UElementAnimatedTexture>(Object);
	if (Texture != nullptr && Texture->GetResource() != nullptr)
	{
		if (Texture->SupportsTransparency)
		{
			// If support transparentcy, draw a checkerboard background first.
			const int32 CheckerDensity = 8;
			UTexture2D* Checker = UThumbnailManager::Get().CheckerboardTexture;
			Canvas->DrawTile(
				0.0f, 0.0f, Width, Height,							// Dimensions
				0.0f, 0.0f, CheckerDensity, CheckerDensity,			// UVs
				FLinearColor::White, Checker->GetResource());			// Tint & Texture
		}

		// Use A canvas tile item to draw
		FCanvasTileItem CanvasTile(FVector2D(X, Y), Texture->GetResource(), FVector2D(Width, Height), FLinearColor::White);
		CanvasTile.BlendMode = Texture->SupportsTransparency ? SE_BLEND_Translucent : SE_BLEND_Opaque;
		CanvasTile.Draw(Canvas);
	}// end of if(texture is valid)
}
