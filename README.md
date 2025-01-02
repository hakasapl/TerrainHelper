# Skyrim Terrain Slot Unlocker

SKSE plugin which allows Skyrim to read more than just diffuse and normal slots for terrain

## Requirements

* [Address Library](https://www.nexusmods.com/skyrimspecialedition/mods/32444)
* [PO3 Tweaks](https://www.nexusmods.com/skyrimspecialedition/mods/51073)

## Mod Authors

Skyrim has a notion of "default landscape texture set". This is usually set in `skyrim.ini` but you can only set diffuse/normal in the ini file. It is by default set to `dirt02.dds` and `dirt02_n.dds`, respectively.

Since this plugin allows for more slots, you will need to define a "default landscape" texture set in a plugin. Just create a texture set and give it the editor ID `LandscapeDefault` and this plugin will automatically use it instead of skyrim.ini as the source. If you do not do this, additional slots will not be used for the default landscape texture, which is used very often in Skyrim (almost everywhere dirt02 exists in-game).

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
* `22` - `27` will be set for any env mask maps available, otherwise slot will not be set
  * `22` = TEX 1 Environment Mask
  * `23` = TEX 2 Environment Mask
  * `24` = TEX 3 Environment Mask
  * `25` = TEX 4 Environment Mask
  * `26` = TEX 5 Environment Mask
  * `27` = TEX 6 Environment Mask

**NOTE** Height and environment mask are mutually exclusive. When both are available env mask will be set. In a single shader materials might be mismatched. For example `TEX 1` could have a height map assigned, where `TEX 2` could have an environment mask assigned, etc.

## Acknowledgements

This plugin would not be possible without the work of the Community Shaders team. Their PBR feature allows for terrain with PBR-relevant slots, so I was able to analyze their code to better understand a method of accomplishing this. The Community Shaders repository can be found [here](https://github.com/doodlum/skyrim-community-shaders). Give them a star!

Thanks to the helpful people on the Community Shaders discord, ENBSeries discord, and the Skyrim RE discord, for helping figure out some concepts needed for this plugin as well as testing.
