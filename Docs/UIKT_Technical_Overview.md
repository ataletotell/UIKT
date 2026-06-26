# UMG Element Kit — Technical Overview

## Architecture

**UMG Element Kit (UIKT)** is a UI component library for Unreal Engine 5 built in three layers:

1. **ELMT Widget Suite** — styled replacements for standard UMG primitives, all following the `UELMTXxxStyle` data asset pattern for global theme control
2. **Advanced Media** — custom `UTexture` subclass with GIF/WebP decoding pipeline; companion `UElementAnimatedImage` widget
3. **Blur Effects** — three blur widgets (shader-masked, native background capture, combined) backed by a custom Slate renderer and render thread pipeline

```
Source/UMGELMTKit/
├── Core/Elements/          ELMT styled widgets
├── Public/                 Animated texture asset, navigation library
├── Public/Components/      Blur widgets + Slate layer
└── Private/                Decoders, render pipeline, libwebp, giflib
```

---

## ELMT Widget Catalog

All ELMT widgets follow one pattern: a `UELMTXxxStyle : public UObject` data asset holds the style struct(s); the widget reads it in `SynchronizeProperties`. Assign a style subclass in the Details panel or via `ExposeOnSpawn`.

### Interactive

| Class | Base | Key additions |
|---|---|---|
| `UELMTButton` | `UButton` | `SimulateHover(bool)` API, CommonUI button style |
| `UELMTIconButton` | `UButton` | Embedded `UImage` icon, 4 press feedback animations (Bounce/Flash/Offset/RotateShake), simulated hover |
| `UELMTSlider` | `USlider` | Style data asset (`FSliderStyle` + focus tint color) |
| `UELMTToggle` | `UCheckBox` | Style data asset (`FCheckBoxStyle`) |
| `UELMTCombo` | `UComboBoxString` | CommonUI button style, `SimulateHover` API |
| `UELMTTextBox` | `UEditableTextBox` | CommonUI button style applied to background images |

**Simulated hover** (`SimulateHover(bool)`) forces a widget into its hovered visual state regardless of cursor position — used to indicate gamepad focus visually.

### Display

| Class | Base | Key additions |
|---|---|---|
| `UELMTProgressBar` | `UProgressBar` | Style data asset (`FProgressBarStyle`) |
| `UELMTBadge` | `UBorder` | Variant system: `Primary/Warning/Danger/Success/Neutral` — call `SetVariant()` to switch color |
| `UELMTSpinner` | `UImage` | Timer-based rotation; `Play()` / `Stop()` / `IsSpinning()`; `bAutoPlay` starts on first game-world `SynchronizeProperties` |
| `UELMTRichText` | `URichTextBlock` | Pulls font/color/shadow from CommonUI `UCommonTextStyle`; optional per-instance color override |

### Navigation & Layout

| Class | Base | Key additions |
|---|---|---|
| `UELMTFocusPanel` | `UContentWidget` | Stops gamepad/keyboard focus from escaping in all 4 directions; `FocusFirstChild()` |
| `UELMTScrollBox` | `UScrollBox` | Style data asset (`FScrollBoxStyle` + `FScrollBarStyle` embedded into `WidgetStyle`) |
| `UELMTTabBar` | `UHorizontalBox` | `SelectTab(int32)`, `SelectNext()`, `SelectPrevious()`, wrap navigation toggle, `OnTabChanged` delegate |

### Composites (Abstract `UUserWidget` bases)

| Class | Base | Pattern |
|---|---|---|
| `UELMTModal` | `UUserWidget` | Subclass in BP; `Confirm()` / `Cancel()` broadcast delegates then call `RemoveFromParent`; override `OnModalConfirmed` / `OnModalCancelled` for close animation; `GetModalFocusPanel()` wires `ELMTFocusPanel` automatically |
| `UELMTToast` | `UUserWidget` | Internal `TArray<FELMTToastMessage>` queue; `ShowMessage()` / `ShowToast()`; override `OnShowToast` / `OnHideToast` for animations; call `NotifyHideComplete()` after hide animation to advance queue |

---

## Animated Texture System

```
UElementAnimatedTexture (UTexture + FTickableGameObject)
    ├── holds raw file blob (TArray<uint8> FileBlob)
    ├── owns TSharedPtr<FElementAnimatedTextureDecoder>
    │       ├── FElementGIFDecoder  (giflib)
    │       └── FElementWebPDecoder (libwebp)
    └── owns FElementAnimatedTextureResource (FTextureResource → RHI)

UElementAnimatedImage (UImage)
    └── holds IndependentTexture instance (Transient, runtime copy)
```

**Tick loop:** `UElementAnimatedTexture::Tick` advances `FrameTime` by `DeltaTime * PlayRate`, checks `FrameDelay`, calls `RenderFrameToTexture()` → `UploadFrameToRHI()` when a new frame is due. The decoder provides raw RGBA pixel data via `GetFrameBuffer()`. A `DecoderLock` `FCriticalSection` guards concurrent access.

**`UElementAnimatedImage`** creates an independent copy of the assigned texture asset (`IndependentTexture`) so multiple image widgets playing the same GIF don't share playhead state.

**API:**

```cpp
// Asset-level (UElementAnimatedTexture)
Play() / PlayFromStart() / Stop() / PlayFromFrame(int32) / SetCurrentFrame(int32)
SetPlayRate(float) / SetLooping(bool) / GetAnimationLength() / IsPlaying()

// Widget-level (UElementAnimatedImage)
SetBrushFromAnimatedTexture(UElementAnimatedTexture*, bool bMatchSize)
Play() / PlayFromFrame(int32) / Stop() / Pause()
```

---

## Blur Effect System

Three widgets exist at different cost/control levels:

### 1. `UElementMaskedBlur` (lightweight, `URetainerBox`-based)

Renders children into a render target, then passes it through `UIBlur.ush` (custom HLSL). The mask texture controls which pixels receive blur.

- **MaskTexture** — grayscale: white = blurred, black = sharp
- **MaxRadius** — blur radius in pixels
- **Quality** — 0 Low (8 taps) → 3 Epic (64 taps)
- **JitterStrength** — dither noise to break up banding

### 2. `UElementMaskedBackgroundBlur` (combined capture + shader mask)

Stacks a native `SBackgroundBlur` (captures gameplay behind the widget) and a `SRetainerWidget` (runs the `UIBlur.ush` material mask) as two Slate layers. Use for frosted-glass effects over gameplay.

Properties split into two groups:
- **Masked Blur**: `EffectMaterial`, `MaskTexture`, `MaxRadius`, `Quality`, `JitterStrength`
- **Background Blur** (native capture): `BlurStrength`, `bApplyAlphaToBlur`, `Padding`

### 3. `UBackgroundBlurWithMask` (full-featured, tick-based)

Custom Slate widget `SBackgroundBlurWithMask` with a full `OnPaint` override. Supports:
- **MaskTexture** or **MaskMaterial** (render-to-texture with `UTextureRenderTarget2D`)
- **MaskTextureChannel** — which RGBA channel drives the mask (R/G/B/A)
- **SetBlurStrengthSmooth(float target, float rate)** — tick-driven lerp for animated blur transitions
- **RedrawMaskMaterial()** — explicit redraw trigger when using `RedrawBlueprint` mode

The backing render pipeline (`BackgroundBlurRenderer`, `BackgroundBlurShaders`, `BackgroundBlurPostProcessor`, `BackgroundBlurDrawer`, `BackgroundBlurPostProcessResource`) runs on the render thread.

```
UBackgroundBlurWithMask (UContentWidget + FTickableGameObject)
    └── SBackgroundBlurWithMask (SCompoundWidget, custom OnPaint)
            └── BackgroundBlurRenderer
                    ├── BackgroundBlurShaders      (HLSL via render thread)
                    ├── BackgroundBlurDrawer
                    └── BackgroundBlurPostProcessor
                            └── BackgroundBlurPostProcessResource
```

---

## Navigation Library

`UELMTNavigationLibrary` (BlueprintFunctionLibrary) — call from BeginPlay or widget construct:

| Function | Effect |
|---|---|
| `SetNavigationStop(Widget, Direction)` | Block focus leaving in one direction |
| `SetNavigationWrap(Widget, Direction)` | Wrap focus at boundary |
| `SetNavigationEscape(Widget, Direction)` | Restore default UE behavior |
| `ContainFocusToWidget(Panel)` | Stop all 4 directions (modal trap) |
| `WrapNavigationOnWidget(Panel)` | Wrap all 4 directions (list wrap) |

`UELMTFocusPanel` calls `SetNavigationStop` on all 4 directions automatically via `bContainFocus` in `SynchronizeProperties`.

---

## Modular Style System

All ELMT primitives use the same pattern:

```
UELMTXxxStyle : public UObject          ← Data Asset (create via right-click → Miscellaneous → Data Asset)
    └── FXxxStyle  StyleStruct          ← Pulled into the widget in SynchronizeProperties
```

This decouples appearance from widget placement. One style asset change updates every widget instance that references it. CommonUI `UCommonButtonStyle` is the shared base for `ELMTButton`, `ELMTIconButton`, `ELMTCombo`, and `ELMTTextBox` — changing the common style cascades across all of them.

---

## UE 5.6 Compatibility Notes

Several UE 5.6 API changes required source-level fixes. See [CHANGELOG.md](../CHANGELOG.md) for the full list.

### Focus Event API Removed from `UWidget` Hierarchy

`NativeOnFocusReceived` and `NativeOnFocusLost` were removed from the `UWidget` base class in UE 5.6. They now exist exclusively on `UUserWidget`. All ELMT classes that inherit from primitive widget types (`USlider`, `UButton`, `UCheckBox`, `UComboBoxString`, `UEditableTextBox`, `UImage`) cannot override these methods.

**Result:** Focus-based visual feedback (simulated hover on focus, handle tint, style swap) is **not active** in UE 5.6 for primitive widget subclasses. The `SimulateHover` API still exists and can be called manually from your own focus logic. To restore automatic behavior, set up focus delegates at the Slate level in `RebuildWidget()` for each widget type.

### Other Breaks (UE 5.6)

| Break | Fix applied |
|---|---|
| `NativeConstruct`/`NativeDestruct` not on `UImage` (`ELMTSpinner`) | Auto-play moved to `SynchronizeProperties`; cleanup to `BeginDestroy` |
| `UWidget::GetDesiredFocusTarget()` removed (`ELMTFocusPanel`) | Returns `GetContent()` directly; no longer recurses |
| `CommonButtonStyle.h` renamed in CommonUI | Changed to `#include "CommonButtonBase.h"` |
| `UScrollBox::ScrollBarStyle` removed | Writes to `WidgetStyle.VerticalScrollbarStyle` / `HorizontalScrollbarStyle` |
| `UComboBoxString::ScrollBarStyle` removed | Assignment removed (no replacement write path on `FComboBoxStyle`) |
| `CommonUI` in private module deps blocked public headers | Moved to `PublicDependencyModuleNames` |

### Known Regressions (UE 5.6)

- Gamepad/keyboard **focus visual feedback** inactive on all primitive widget subclasses.
- `ELMTFocusPanel::bAutoFocusFirstChild` — no `NativeConstruct` on `UContentWidget`. Call `FocusFirstChild()` explicitly from the owning `UUserWidget::NativeConstruct`.
