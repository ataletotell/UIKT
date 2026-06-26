# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [1.1.0] - 2026-06-23

### Changed (UE 5.6 Port)

- **Focus system removed from primitive widget subclasses.** UE 5.6 removed `NativeOnFocusReceived` and `NativeOnFocusLost` from the `UWidget` hierarchy. These virtual methods now exist only on `UUserWidget`. All ELMT classes that inherit from primitive widgets (`USlider`, `UButton`, `UCheckBox`, `UComboBoxString`, `UEditableTextBox`, `UImage`) no longer override focus events.
  - Affected: `ELMTSlider`, `ELMTButton`, `ELMTIconButton`, `ELMTCombo`, `ELMTTextBox`, `ELMTToggle`
- **`ELMTSpinner` lifecycle replaced.** `NativeConstruct`/`NativeDestruct` were `UUserWidget`-only APIs. Auto-play now triggers inside `SynchronizeProperties` (game world only, guarded against re-entry). Cleanup uses `BeginDestroy` instead of `NativeDestruct`.
- **`ELMTFocusPanel` focus delegation simplified.** `UWidget::GetDesiredFocusTarget()` was removed in 5.6. `GetDesiredFocusTarget()` now returns the direct content widget rather than recursing into it. `NativeConstruct` override removed; navigation rules still apply via `SynchronizeProperties`.
- **`CommonButtonStyle.h` include replaced.** Header was renamed/moved in CommonUI for UE 5.6. All includes now use `CommonButtonBase.h` which transitively provides `UCommonButtonStyle`.
- **`ScrollBarStyle` removed from `UScrollBox`.** In UE 5.6, `UScrollBox` no longer exposes a top-level `ScrollBarStyle` property. `ELMTScrollBox` now writes the scroll bar style into `WidgetStyle.VerticalScrollbarStyle` and `WidgetStyle.HorizontalScrollbarStyle`.
- **`ScrollBarStyle` removed from `UComboBoxString`.** No equivalent write-path exists on `FComboBoxStyle`. `ELMTCombo` scroll bar style assignment has been removed; the combo dropdown uses the engine default scroll bar style.
- **Build.cs**: `UMG` and `CommonUI` moved from `PrivateDependencyModuleNames` to `PublicDependencyModuleNames` so that modules depending on UIKT public headers can resolve CommonUI and UMG types without additional setup.

### Known Regressions (UE 5.6)

- Gamepad/keyboard **focus visual feedback** is inactive for all primitive widget subclasses (handle tint on `ELMTSlider`, simulated hover on `ELMTButton`/`ELMTIconButton`/`ELMTCombo`, style swap on `ELMTToggle`, background change on `ELMTTextBox`). These require a Slate-level delegate approach to re-implement under the new API.
- `ELMTFocusPanel::bAutoFocusFirstChild` no longer triggers automatically on widget construction (no `NativeConstruct` hook available on `UContentWidget`). Call `FocusFirstChild()` explicitly from the owning `UUserWidget::NativeConstruct` instead.

## [1.0.0] - 2026-04-18
### Added
- **ELMT Widget Suite**: Modular set of enhanced UMG widgets including `ELMTCombo`, `ELMTProgressBar`, and `ELMTRichText`.
- **Animated Texture Support**: Custom decoding system for `GIF` and `WebP` formats in UMG.
- **Common UI Integration**: Deep integration with Epic's Common UI for cross-platform input and styling.
- **Modular Styling System**: Centralized "Theme" support for global aesthetic updates across all `ELMT` components.
- **Asynchronous Decoding**: Performance-optimized background decoding for animated UI assets.
- **Custom DPI Scaling**: Advanced DPI curve implementation for desktop-style app behavior.
- **Documentation**: Professional technical overview and walkthrough guides.

### Changed
- Refined `ELMTRichText` to support dynamic style overrides and project-wide typography settings.
- Standardized plugin structure for Unreal Engine 5.6 compatibility.
