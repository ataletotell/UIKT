# UMG Element Kit - Walkthrough

## Overview
A modern toolkit for building high-fidelity user interfaces. UMG Element Kit (UIKT) provides a full library of stylized widgets and advanced media features like GIF support to help you build interfaces that look and feel premium.

## Key Features

- **Stylized "ELMT" Widgets**: Upgraded versions of standard UMG components with better visual control.
- **GIF & WebP Support**: Play animated images directly in your UI with high performance.
- **Common UI Ready**: Full support for gamepads and multi-layered UI stacks.
- **Modular & Themeable**: Update the look of your entire game from a single location.

## How to Use

### 1. Using ELMT Widgets
1. In your Widget Blueprint, look for the **Element Kit** category in the Palette.
2. Drag in an **ELMT Rich Text**, **ELMT Progress Bar**, or **ELMT Button**.
3. Use the **Appearance** section in the Details panel to configure colors, padding, and glows without needing complex custom shaders.

### 2. Playing GIFs and WebP Images
1. Import a `.gif` or `.webp` file as a regular texture (the plugin handles the conversion).
2. Add an **Element Animated Image** widget to your UI.
3. Select your animated asset in the widget settings.
4. Call `Play` or `Stop` from your Blueprint logic to control the animation.

### 3. Integrating with Common UI
Because `UIKT` widgets inherit from Common UI base classes, they automatically support:
- **Focus Management**: Navigate buttons with a gamepad or keyboard.
- **Input Icons**: Automatically show Xbox/PlayStation icons based on the connected controller.

### 4. Advanced Styling
Use the `ELMTRichText` widget to create complex text layouts. You can use tags like `<bold>`, `<glow>`, or `<link>` to create interactive and visually striking text without creating multiple text blocks.

## Tips
- **Performance**: While GIFs are convenient, large ones can consume memory. Use the **WebP** format where possible as it offers better compression.
- **Accessibility**: Use the built-in "Hover" and "Pressed" states on ELMT buttons to ensure clear visual feedback for all players.
