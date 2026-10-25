# Torghast ModelScene compatibility for WarcraftXL

This WXL mod supplies the Retail-shaped `GlobalFXBackgroundModelScene`,
`GlobalFXMediumModelScene`, and `GlobalFXHighModelScene` globals in the game's 3.3.5 Lua VM.
Effects are real WotLK `Model` frames, with controller methods (`Stop`, `SetPaused`,
`IsPlaying`, `SetModelScale`, and `SetAlpha`).

## Registering effect models

Retail effect IDs identify Retail model-scene records. Retail M2 assets and those records are not
directly loadable by the 3.3.5 model reader. A companion 3.3.5 addon must register converted or
WotLK-native model files before calling `AddEffect`:

```lua
WXLModelSceneCompat.RegisterEffect(89, "Interface\\AddOns\\MythicPlus\\Effects\\TorghastSmoke.m2", {
    scale = 1,
    alpha = 0.9,
})
WXLModelSceneCompat.RegisterEffect(95, "Interface\\AddOns\\MythicPlus\\Effects\\TorghastPowerSwirl.m2", {
    scale = 1,
    alpha = 1,
})

local controller = GlobalFXBackgroundModelScene:AddEffect(89, TorghastLevelPickerFrame, {
    duration = 8,
    scale = 1,
})
```

No model assets are included yet: the Retail effect records/assets need extraction and conversion
to a format this client can parse. This layer deliberately does not substitute atlas animations
for those 3D effects. The picker addon should call `controller:Stop()` when it hides or changes
pages. Loading this mod is separate from the addon: build it with the adjacent SDK, install its
`dist/wxl.torghast-modelscene` folder into `client/bin/Win/Mods`, and enable the companion addon.

## Build

Needs a checkout of the WarcraftXL SDK ([Tyrallis/sdk](https://github.com/Tyrallis/sdk/tree/x64-port), `x64-port`) at
`../../sdk`, or pass `-DWXL_SDK_DIR=...`. Build with Visual Studio 2022: the 2026 Insiders compiler (MSVC 14.50) crashes
on the SDK's headers. Since 0.2.0 the mod needs WarcraftXL ABI 0.1.15; older clients refuse it.

```
cmake -S . -B build
cmake --build build --config Release
```

Copy `build\dist\wxl.torghast-modelscene\` into `<client>\bin\Win\Mods\`.

## License

GPL-3.0, see `LICENSE`.
