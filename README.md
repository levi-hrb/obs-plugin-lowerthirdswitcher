# obs-plugin-lowerthirdswitcher
An OBS Plugin to easily display pre-configured lower thirds.

**Version 2.0** - Updated to use the latest OBS Plugin Template (2025) with support for OBS Studio 31.0+ and Qt6.

<img width="1440" alt="Screenshot 2024-01-23 at 14 07 45" src="https://github.com/levi-hrb/obs-plugin-lowerthirdswitcher/assets/65330078/7302f64f-dc64-4e1f-94b1-5c18431ab069">

<br><br>
Say thanks with a chocolate bar :)<br>
<a href="https://www.paypal.com/donate/?hosted_button_id=7K3HTSKYHPHWG">
    <img width="100" src="https://www.paypalobjects.com/webstatic/de_DE/i/de-pp-logo-200px.png" alt="PayPal Button">
</a>


## Description
Create an OBS folder that includes a text source for the main text (e.g. names) and one for the secondary subtitle (e.g. titles). The folder can also include any other design elements. 

<img width="200" alt="Sources Screenshot" src="https://github.com/levi-hrb/obs-plugin-lowerthirdswitcher/assets/65330078/c9eed82a-a017-45ae-984d-ec85eaa7f4ea">

In the OBS Docks menu, the "Lower Third Switcher" dock can be enabled. In the settings tab the scene, folder source, main text source and secondary source can be selected. The "Display Time" defines how long the lower third will be visible. If the "Display Time" is set to 0, the lower third gets triggered and becomes visible, but never hides.

<img width="300" alt="Settings Screenshot" src="https://github.com/levi-hrb/obs-plugin-lowerthirdswitcher/assets/65330078/552ca258-08f7-4ab0-b3b5-22a0a67c2ca1">

After the settings are set up, you can switch to the data tab in the Lower Third Switcher dock. For each lower third, you would like to show throughout the stream you can create a new entry. The item that has a green dot is the active item, which will be shown next.

<img width="300" alt="Data Screenshot" src="https://github.com/levi-hrb/obs-plugin-lowerthirdswitcher/assets/65330078/b6c1d538-e1d2-46e7-bf4e-c5d3f68bf292">

During the stream/recording, you can show the next (active) item by clicking the "Play Next Lower Third" button, this action will also set the next item in line as active. A flashing red dot will indicate that the lower third is currently visible. 
A hotkey can be set up to show the next lower third. This enables many opportunities to control the plugin, e.g. a midi device can be connected and with a plugin like [obs-midi-mg](https://github.com/nhielost/obs-midi-mg/) midi signals can trigger the hotkey, thus the midi device can trigger the next lower third to be shown.

## Installing
Go to the [Releases page](https://github.com/levi-hrb/obs-plugin-lowerthirdswitcher/releases) and download and install the latest release for the proper operating system. After installing the plugin, you will find it in OBS in the "Docks" menu.

## Building from Source

### Prerequisites
- CMake 3.28 or newer
- Xcode (macOS) or Visual Studio (Windows)
- OBS Studio 31.0 or newer (automatically downloaded during build)
- Qt6 (automatically downloaded during build)

### macOS Build Instructions
```bash
# Clone the repository
git clone https://github.com/anduloo/obs-plugin-lowerthirdswitcher.git
cd obs-plugin-lowerthirdswitcher

# Configure the project
cmake -S . -B build -G Xcode -DCMAKE_OSX_ARCHITECTURES="arm64;x86_64"

# Build the plugin
cmake --build build --config Release

# The plugin will be located at:
# build/Release/obs-plugin-lowerthirdswitcher.plugin
```

### Installing the Built Plugin
Copy the built plugin to your OBS plugins directory:
```bash
# User-specific installation
cp -r build/Release/obs-plugin-lowerthirdswitcher.plugin ~/Library/Application\ Support/obs-studio/plugins/

# System-wide installation (requires sudo)
sudo cp -r build/Release/obs-plugin-lowerthirdswitcher.plugin /Library/Application\ Support/obs-studio/plugins/
```

## Development
This plugin uses the [OBS Plugin Template](https://github.com/obsproject/obs-plugintemplate) as its foundation. The build system automatically downloads required dependencies including OBS Studio sources, Qt6, and obs-deps.

### Project Structure
- `src/` - Source code files
- `data/` - Plugin resources (icons, UI files)
- `cmake/` - CMake build configuration
- `buildspec.json` - Dependency specification
