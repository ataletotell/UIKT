# UMG Element Kit — Walkthrough

## Contents

1. [Setup](#setup)
2. [ELMT Interactive Widgets](#elmt-interactive-widgets)
3. [ELMT Display Widgets](#elmt-display-widgets)
4. [ELMT Navigation & Layout](#elmt-navigation--layout)
5. [Modal Dialogs](#modal-dialogs)
6. [Toast Notifications](#toast-notifications)
7. [Tab Bars](#tab-bars)
8. [Animated Images (GIF/WebP)](#animated-images-gifwebp)
9. [Blur Effects](#blur-effects)
10. [Navigation Library](#navigation-library)
11. [Common UI Integration](#common-ui-integration)
12. [UE 5.6 Notes](#ue-56-notes)

---

## Setup

Add `UIKT` to your game's `Build.cs`:

```csharp
PrivateDependencyModuleNames.AddRange(new string[] { "UMGELMTKit" });
```

All ELMT widget classes are in the `ELMT` category in the UMG Palette.

---

## ELMT Interactive Widgets

All interactive widgets share the same styling pattern:

1. Create a Data Asset: right-click in Content Browser → **Miscellaneous → Data Asset**
2. Select the matching `UELMTXxxStyle` class
3. Fill out the style struct fields (fonts, brush textures, colors)
4. Assign the style class in the widget's **Details → Style** property

### ELMTButton

Drop-in replacement for `UButton`. Exposes `SimulateHover(bool)` to force hover visuals from Blueprint — use this to indicate gamepad focus.

**Blueprint usage:**

```
On Focused → ELMTButton → SimulateHover (true)
On Unfocused → ELMTButton → SimulateHover (false)
```

### ELMTIconButton

Like `ELMTButton` but adds an embedded icon `UImage` and four press feedback animation modes:

| Mode | Effect |
|---|---|
| `Bounce` | Scale down on press, spring back on release |
| `Flash` | Briefly flash opacity |
| `Offset` | Translate icon by press offset |
| `RotateShake` | Shake rotation on press |

Set `PressAnimation` in the style asset. All animations are timer-based, no Sequencer needed.

### ELMTSlider

Styled slider. Style asset holds `FSliderStyle` plus a `FocusHandleColor` (tint applied on focus, UE 5.6: inactive — see [UE 5.6 Notes](#ue-56-notes)).

### ELMTToggle

Styled checkbox/toggle. Style asset holds `FCheckBoxStyle` (checked/unchecked brush images).

### ELMTCombo

Styled combo box. Uses CommonUI button style for the collapsed button. `SimulateHover(bool)` available for gamepad focus indication.

### ELMTTextBox

Styled editable text box. Uses CommonUI button style textures for the background field visuals.

---

## ELMT Display Widgets

### ELMTProgressBar

Style data asset holds `FProgressBarStyle` (fill brush, background brush, border padding). Works identically to `UProgressBar` at the API level.

### ELMTBadge

Border widget with a variant color system. Set `Variant` in the Details panel or call `SetVariant(EELMTBadgeVariant)` at runtime:

| Variant | Intent |
|---|---|
| `Primary` | Brand highlight |
| `Success` | Positive state |
| `Warning` | Caution |
| `Danger` | Error / destructive |
| `Neutral` | Default / informational |

Style asset maps each variant to a `FLinearColor`.

### ELMTSpinner

Rotating image widget. Style asset holds the spinner texture, spin speed (degrees/sec), size, and tint.

- `Play()` — start spinning
- `Stop()` — stop and reset to 0°
- `IsSpinning()` — current state

`bAutoPlay = true` (default) auto-starts on first `SynchronizeProperties` call in a game world.

### ELMTRichText

`URichTextBlock` with CommonUI style integration. Style asset holds:
- `CommonButtonStyle` (optional) — pulls font/color from CommonUI button style
- `TextStyle` — base `FTextBlockStyle`

Per-instance overrides:
- `OverwriteTextStyle` — CommonUI text style reference for quick swap
- `bOverrideTextColor` + `TextColorOverride` — inline color override without touching the style asset

Supports all standard rich text markup (`<b>`, `<i>`, custom row decorators).

---

## ELMT Navigation & Layout

### ELMTFocusPanel

Content widget that stops gamepad/keyboard navigation from leaving in all four directions. Use as root panel of modals, sidebars, or any overlay that should trap focus.

**Usage in C++:**

```cpp
// Cannot use NativeConstruct (UContentWidget). Call from owning UUserWidget:
void UMyModal::NativeConstruct()
{
    Super::NativeConstruct();
    if (FocusPanel)
    {
        FocusPanel->FocusFirstChild();
    }
}
```

**Usage in Blueprint:**

```
Event NativeConstruct → FocusPanel → FocusFirstChild
```

### ELMTScrollBox

Styled scroll box. Style asset holds both `FScrollBoxStyle` and scroll bar style fields which are written into `WidgetStyle.VerticalScrollbarStyle` / `HorizontalScrollbarStyle` (UE 5.6 path).

---

## Modal Dialogs

`UELMTModal` is an abstract `UUserWidget` base. Subclass it in Blueprint:

1. Create Widget Blueprint → parent class: **ELMTModal**
2. Design your layout (title text, confirm button, cancel button, background panel)
3. Wire confirm button `OnClicked` → **Confirm**
4. Wire cancel button `OnClicked` → **Cancel**
5. Override `OnModalConfirmed` / `OnModalCancelled` to drive close animations
6. Override `GetModalFocusPanel` to return the `ELMTFocusPanel` wrapping your content — focus moves into it automatically on construct

**Spawning from Blueprint:**

```
Create Widget (your Modal subclass)
→ Add to Viewport (ZOrder high)
→ Bind OnConfirmed / OnCancelled delegates
```

**Confirm/Cancel flow:**

`Confirm()` → broadcasts `OnConfirmed` → calls `OnModalConfirmed` (BP override, play close anim here) → `RemoveFromParent`

`Cancel()` → broadcasts `OnCancelled` → calls `OnModalCancelled` → `RemoveFromParent`

**Tip:** If your close animation needs time before `RemoveFromParent`, override `OnModalConfirmed`, play the anim, and on the animation's `OnFinished` event call `RemoveFromParent` manually. Return early from the default super call if needed.

---

## Toast Notifications

`UELMTToast` is an abstract `UUserWidget` base with an internal message queue. Subclass in Blueprint:

1. Create Widget Blueprint → parent class: **ELMTToast**
2. Design your toast visual (text block, icon, background)
3. Override `OnShowToast(Toast)` — update text/color from the `Toast` struct, play show animation
4. Override `OnHideToast` — play hide animation; when animation finishes call **NotifyHideComplete**

**Enqueueing messages:**

```
// Simple text
ToastWidget → ShowMessage ("Saved!", 2.0)

// With variant
Toast Message struct: Message="Achievement unlocked", Duration=4.0, Variant=Success
ToastWidget → ShowToast (Toast)
```

Queue behavior: toasts play one at a time. After `Duration` seconds, `OnHideToast` fires. After you call `NotifyHideComplete`, the next queued message shows. Call `ClearQueue()` to abort all pending toasts.

**Recommended setup:** add the toast widget as a persistent overlay in your root HUD widget. Keep one instance for the session lifetime.

---

## Tab Bars

`UELMTTabBar` extends `UHorizontalBox`. Add `ELMTButton` children as tabs.

**Setup:**

1. Add an **ELMT Tab Bar** to your widget
2. Add **ELMT Button** (or ELMT Icon Button) children in the Hierarchy for each tab
3. Wire tab buttons to call `SelectTab(index)` on click
4. Bind `OnTabChanged` delegate to swap displayed content panels

**Navigation:**

```
// Gamepad shoulder buttons
InputAction TabRight → ELMTTabBar → SelectNext
InputAction TabLeft  → ELMTTabBar → SelectPrevious
```

`bWrapNavigation = true` (default) wraps from last tab to first and vice-versa.

`GetActiveTabIndex()` returns current 0-based index. `GetTabCount()` returns child button count.

---

## Animated Images (GIF/WebP)

### Importing

1. Drag `.gif` or `.webp` into Content Browser
2. Plugin creates a `UElementAnimatedTexture` asset automatically (via `ElementAnimatedTextureFactory` in the editor module)
3. Thumbnail renders the first frame via `ElementAnimatedTextureThumbnailRenderer`

### Playing in UI

Add **Element Animated Image** widget. Assign the animated texture asset. With `bAutoPlay = true` the animation starts when the widget is displayed.

**Runtime control:**

```
AnimImage → Play
AnimImage → Stop
AnimImage → Pause
AnimImage → PlayFromFrame (3)

// Swap texture at runtime:
AnimImage → SetBrushFromAnimatedTexture (NewTexture, true)
```

Multiple `ElementAnimatedImage` widgets can reference the same asset — each gets its own independent playhead (`IndependentTexture`).

### Asset-level control

If you need to control the source `UElementAnimatedTexture` asset directly (e.g., sync animation across all widgets sharing it):

```
AnimTexture → PlayFromStart
AnimTexture → SetPlayRate (2.0)
AnimTexture → SetLooping (false)
AnimTexture → GetAnimationLength → (float seconds)
```

**Performance tip:** WebP offers significantly better compression than GIF at equivalent quality. Prefer WebP for UI animations in shipped content.

---

## Blur Effects

Three widgets at different cost tiers. Pick based on what you need to blur.

### `ElementMaskedBlur` — blur UI children

Use when you want to blur widgets **inside** the widget, not behind it. It's a `URetainerBox` — children render into a texture, then the `UIBlur.ush` shader applies masked blur.

```
ElementMaskedBlur
└── [child widgets to blur]
```

**Key properties:**

| Property | Description |
|---|---|
| `MaskTexture` | Grayscale mask: white = blurred, black = sharp |
| `MaxRadius` | Blur radius in pixels |
| `Quality` | 0 = 8 taps (fastest), 3 = 64 taps (sharpest) |
| `JitterStrength` | Dither noise to reduce banding (0–1) |

Runtime setters available: `SetMaskTexture`, `SetMaxRadius`, `SetQuality`, `SetJitterStrength`.

### `ElementMaskedBackgroundBlur` — frosted glass over gameplay

Blurs gameplay **behind** the widget (like native `UBackgroundBlur`) AND applies a custom shader mask. Use for shaped frosted-glass overlays (glasses lenses, frosted panels).

Requires an `EffectMaterial` using the `UIBlur.ush` shader with mask texture input.

```
ElementMaskedBackgroundBlur
└── [optional UI content on top of blur]
```

**Properties:**

```
Masked Blur group:   EffectMaterial, MaskTexture, MaxRadius, Quality, JitterStrength
Background Blur group: BlurStrength (0-100), bApplyAlphaToBlur, Padding
```

### `BackgroundBlurWithMask` — full-featured with material and smooth transitions

Most powerful option. Full custom Slate render pipeline. Tick-based smooth blur transitions.

```
BackgroundBlurWithMask
└── [child widget (optional)]
```

**Key features over the other two:**

- **`SetBlurStrengthSmooth(float target, float rate)`** — tick-interpolated blur strength, no animation setup needed
- **`MaskTextureChannel`** — choose R, G, B, or A channel as blur mask
- **`bUseMaskMaterial` + `MaskMaterialSetting`** — render-to-texture mask via material (use for animated masks)
- **`RedrawMaskMaterial()`** — manually trigger mask material redraw
- **`LowQualityFallbackBrush`** — fallback image when `Slate.ForceBackgroundBlurLowQualityOverride 1` is set in scalability

**Smooth blur example (transition in/out):**

```
// Fade blur in when menu opens
OnMenuOpen → BackgroundBlurWithMask → SetBlurStrengthSmooth (30.0, 8.0)

// Fade blur out when menu closes
OnMenuClose → BackgroundBlurWithMask → SetBlurStrengthSmooth (0.0, 8.0)
```

---

## Navigation Library

`UELMTNavigationLibrary` — call from `NativeConstruct` or `BeginPlay` to configure focus boundaries.

```
// Trap focus inside a panel (all 4 directions stopped):
ELMT Navigation → ContainFocusToWidget (MyPanel)

// Wrap focus on a horizontal list:
ELMT Navigation → WrapNavigationOnWidget (MyListBox)

// Stop navigation in one direction only:
ELMT Navigation → SetNavigationStop (MyButton, Left)

// Restore default UE behavior for one direction:
ELMT Navigation → SetNavigationEscape (MyButton, Right)
```

**Modal + FocusPanel + NavigationLibrary together:**

```
NativeConstruct:
    ELMT Navigation → ContainFocusToWidget (ModalRoot)
    ModalFocusPanel → FocusFirstChild
```

This traps gamepad focus inside the modal until `Confirm`/`Cancel` removes it.

---

## Common UI Integration

UIKT uses CommonUI styling as its shared type system. `UCommonButtonStyle` drives visual appearance across `ELMTButton`, `ELMTIconButton`, `ELMTCombo`, `ELMTTextBox`, and `ELMTRichText`. `UCommonTextStyle` feeds `ELMTRichText` for typography.

**Shared style approach:**

Create one `UCommonButtonStyle` Data Asset for each visual tier (e.g., `DA_Style_Primary`, `DA_Style_Secondary`, `DA_Style_Danger`). Reference these in your ELMT style assets. All ELMT widgets using the same Common Button Style automatically update when you change it.

**Input icons:** unchanged in UE 5.6. Xbox/PlayStation icons resolve automatically from the connected input device.

---

## UE 5.6 Notes

### Focus visual feedback inactive on primitive widget subclasses

`NativeOnFocusReceived`/`NativeOnFocusLost` were removed from `UWidget` in UE 5.6. ELMT classes that extend `USlider`, `UButton`, `UCheckBox`, `UComboBoxString`, `UEditableTextBox`, and `UImage` no longer receive focus events.

**Impact:** handle tint on `ELMTSlider`, simulated hover on `ELMTButton`/`ELMTIconButton`/`ELMTCombo`, style-swap on `ELMTToggle`, background change on `ELMTTextBox` are all inactive.

**Workaround:** Bind focus events at the calling widget level and invoke `SimulateHover` manually:

```
// In your owning UUserWidget or Input Component:
MyButton → OnFocusReceived → ELMTButton → SimulateHover (true)
MyButton → OnFocusLost    → ELMTButton → SimulateHover (false)
```

### FocusPanel auto-focus requires explicit call

`UELMTFocusPanel` has no `NativeConstruct` (it's a `UContentWidget`, not `UUserWidget`). Call `FocusFirstChild()` manually from the owning widget:

```cpp
void UMyOverlay::NativeConstruct()
{
    Super::NativeConstruct();
    FocusPanel->FocusFirstChild();
}
```

### Combo scroll bar style lost

`UComboBoxString::ScrollBarStyle` removed with no write-back path in UE 5.6. `ELMTCombo` dropdown uses engine default scroll bar style. Workaround: override the engine's default `ComboBoxStyle` in project UI settings.
