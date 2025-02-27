# Terrain Helper

[![Build Status](https://github.com/hakasapl/SkyrimTerrainSlotUnlocker/actions/workflows/build.yml/badge.svg?branch=main)](https://github.com/hakasapl/SkyrimTerrainSlotUnlocker/actions/workflows/build.yml)

SKSE plugin which allows Skyrim to read more than just diffuse and normal slots for terrain

## Requirements

* [Address Library](https://www.nexusmods.com/skyrimspecialedition/mods/32444)
* [PO3 Tweaks](https://www.nexusmods.com/skyrimspecialedition/mods/51073)

## Mod Authors

Skyrim has a notion of "default landscape texture set". This is usually set in `skyrim.ini` but you can only set diffuse/normal in the ini file. It is by default set to `dirt02.dds` and `dirt02_n.dds`, respectively. Since you probably want extra slots for the default set too, this plugin reads the texture set with the editor ID `LandscapeDefault` to find them. This is included in `TerrainHelper.esp`. In your mod you should override this record and ensure `TerrainHelper.esp` is a master. This will also be another hint for users that this mod is required for those that miss it.

Other than that, all you need to do is override vanilla TXST records that are referenced by LTEX records and this plugin will pick it up, assuming you are using one of the compatible slots - see below. Parallax in the alpha of the diffuse is no longer needed!

## Technical Details

This plugin replaces the landscape shader material with a custom one that implements additional slots. As such, it is compatible with everything, but by its own will do nothing visually. It is up to ENB and/or CS to make use of the additional slots allocated to the shader.

These are all the slots in the extended terrain shader:

* `0` - `15` are the same as they were in vanilla
  * `0` = TEX 1 Diffuse
  * `1` = TEX 2 Diffuse
  * `2` = TEX 3 Diffuse
  * `3` = TEX 4 Diffuse
  * `4` = TEX 5 Diffuse
  * `5` = TEX 6 Diffuse
  * `6` = Unused / Black
  * `7` = TEX 1 Normal
  * `8` = TEX 2 Normal
  * `9` = TEX 3 Normal
  * `10` = TEX 4 Normal
  * `11` = TEX 5 Normal
  * `12` = TEX 6 Normal
  * `13` = Terrain Overlay Texture
  * `14` = Unknown
  * `15` = Terrain Noise Texture
* `16` - `21` will be set for any height maps available, otherwise slot will not be set
  * `16` = TEX 1 Height
  * `17` = TEX 2 Height
  * `18` = TEX 3 Height
  * `19` = TEX 4 Height
  * `20` = TEX 5 Height
  * `21` = TEX 6 Height

## Acknowledgements

This plugin would not be possible without the work of the Community Shaders team. Their PBR feature allows for terrain with PBR-relevant slots, so I was able to analyze their code to better understand a method of accomplishing this. The Community Shaders repository can be found [here](https://github.com/doodlum/skyrim-community-shaders). Give them a star!

Thanks to the helpful people on the Community Shaders discord, ENBSeries discord, and the Skyrim RE discord, for helping figure out some concepts needed for this plugin as well as testing.

Thanks to Boris and ENBseries discord for testing and integrating this into ENBSeries.
