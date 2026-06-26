# ElementUI UMG Kit - UE5 Plugin

**An easy way to change the style of your elements that extends beyond the default ones the Common UI provides.**

![alt text](https://i.imgur.com/tvVaHcT.png)

_\*Ready to use, drag & drop_

## Info

These are not Elements wrapped in a Widget Blueprint exposing _some_ functionality one by one. These are elements that extend the default ones allowing a `TextBox` for example to inherit the style from _CommonUI Button Style_ while keeping full access to all the default properties.

This allows for easy changes to the style, some of the elements are based on others for example, changing `ElementUI_Style_Btn_Primary` will change the style for all of them, while these keep their respective colors.

## How to use it?

Search for `ElementUI_` in your Blueprint Widget

<details>
  <summary>Preview elements</summary>

![alt text](https://i.imgur.com/nqDP0dq.png)

</details>

<details>
  <summary>Preview edit</summary>

![alt text](https://i.imgur.com/F8lKlpj.png)

</details>

## How to change the styles?

In your blueprint widget, click an element and it will have a _"Style"_ property that can be changed and allow only styles specific to that element.
Or to create a new one besides the Common UI Style, search for `ELMT`

See `UMGELMTKit/Content/Interface/Style/`

## Styles Inheritance order

**Buttons**

```
UMGELMTKit/Content/Interface/Style/Buttons/ElementUI_Style_Btn_Primary
├─ ElementUI_Style_Btn_*
└─ ElementUI_Style_Btn_Secondary
   ├─ ElementUI_Style_Btn_Borderless
   └─ ElementUI_Style_Btn_Outline
      ├─ ElementUI_Style_TextBox
      └─ ElementUI_Style_Combo
```

**Normal Text**

```
UMGELMTKit/Content/Interface/Style/Text/ElementUI_Style_Text_Normal
├─ ElementUI_Style_Text_Normal_Gray
└─ ElementUI_Style_Text_Normal_Gray
```

**Small Text**

```
UMGELMTKit/Content/Interface/Style/Text/ElementUI_Style_Text_Small
├─ ElementUI_Style_Text_Small_Gray
└─ ElementUI_Style_Text_Small_Gray
```



## ELMT Widget Suite

Source class prefix: `UULMT` / Blueprint palette category: **ELMT**

### Interactive

| Class | Based on | Adds |
|---|---|---|
| `ELMTButton` | `UButton` | CommonUI style, `SimulateHover(bool)` |
| `ELMTIconButton` | `UButton` | Icon image, 4 press animations (Bounce/Flash/Offset/RotateShake), `SimulateHover` |
| `ELMTSlider` | `USlider` | Style data asset |
| `ELMTToggle` | `UCheckBox` | Style data asset |
| `ELMTCombo` | `UComboBoxString` | CommonUI style, `SimulateHover` |
| `ELMTTextBox` | `UEditableTextBox` | CommonUI style on background images |

### Display

| Class | Based on | Adds |
|---|---|---|
| `ELMTProgressBar` | `UProgressBar` | Style data asset |
| `ELMTBadge` | `UBorder` | Variant color system (Primary/Success/Warning/Danger/Neutral) |
| `ELMTSpinner` | `UImage` | Timer-driven rotation, `Play/Stop`, `bAutoPlay` |
| `ELMTRichText` | `URichTextBlock` | CommonUI text style integration, per-instance color override |

### Navigation & Layout

| Class | Based on | Adds |
|---|---|---|
| `ELMTFocusPanel` | `UContentWidget` | Traps gamepad/keyboard focus in all 4 directions |
| `ELMTScrollBox` | `UScrollBox` | Style data asset |
| `ELMTTabBar` | `UHorizontalBox` | `SelectTab/Next/Previous`, `OnTabChanged` delegate, wrap toggle |

### Composites (Blueprint abstract bases)

| Class | Description |
|---|---|
| `ELMTModal` | Modal dialog base. `Confirm/Cancel` with delegates + auto-remove. Override `OnModalConfirmed/Cancelled` for animations. |
| `ELMTToast` | Toast notification with internal queue. `ShowMessage/ShowToast/ClearQueue`. Override `OnShowToast/OnHideToast`. |

---

## Advanced Media

**Animated Texture (GIF/WebP)**

Import `.gif` or `.webp` — plugin creates a `UElementAnimatedTexture` asset using giflib/libwebp decoders. Each frame uploads to RHI via `FTickableGameObject::Tick`.

- `ElementAnimatedImage` widget — wraps `UImage`, creates independent playhead per instance
- Full playback API: `Play/Stop/Pause/PlayFromFrame/SetPlayRate/SetLooping/GetAnimationLength`

---

## Blur Effects

Three widgets at increasing cost/control levels:

| Widget | Based on | Blurs | Mask |
|---|---|---|---|
| `ElementMaskedBlur` | `URetainerBox` | Children inside widget | Texture (UIBlur.ush shader) |
| `ElementMaskedBackgroundBlur` | `UContentWidget` | Gameplay behind widget | Texture + material |
| `BackgroundBlurWithMask` | `UContentWidget` + `FTickableGameObject` | Gameplay behind widget | Texture or animated material, per-channel selection |

`BackgroundBlurWithMask` adds `SetBlurStrengthSmooth(target, rate)` for tick-driven blur transitions without animations.

---

## Navigation Library

`UELMTNavigationLibrary` — Blueprint function library for setting navigation stop/wrap/escape rules on any widget:

- `ContainFocusToWidget(Panel)` — stop all 4 directions (modal trap)
- `WrapNavigationOnWidget(Panel)` — wrap all 4 (list wrap)
- `SetNavigationStop/Wrap/Escape(Widget, Direction)` — per-direction control

---

## Components

Prefix: `ElementUI_C_`

Blueprint widget components built from ELMT elements:

<details>
  <summary>Alert Dialog</summary>

    UMGELMTKit/Content/Interface/Components/ElementUI_C_AlertDialog

![alt text](https://i.imgur.com/ZtCLHC6.png)

</details>

<details>
  <summary>CheckBox Label</summary>

    UMGELMTKit/Content/Interface/Components/ElementUI_C_CheckboxLabel

![alt text](https://i.imgur.com/YKAvQbg.png)

</details>

## Extra

- Custom DPI Scale that matches some desktop app behavior (see UMGELMTKit/Content/Interface/ElementUI_DPI_Curve)

- 3 Demo Widgets including this Email Dashboard inspired by [shadcn](https://ui.shadcn.com/examples/mail):

![alt text](https://i.imgur.com/U7s8kmO.png)

## Documentation

*   [Technical Overview](Docs/UIKT_Technical_Overview.md)
*   [Walkthrough](Docs/UIKT_Walkthrough.md)
*   [Changelog](CHANGELOG.md)
