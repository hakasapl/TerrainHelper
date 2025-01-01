# Skyrim Terrain Slot Unlocker

SKSE plugin which allows Skyrim to read more than just diffuse and normal slots for terrain

## Requiredments

* [Address Library](https://www.nexusmods.com/skyrimspecialedition/mods/32444)
* [PO3 Tweaks](https://www.nexusmods.com/skyrimspecialedition/mods/51073)

## Mod Authors

Skyrim has a notion of "default landscape texture set". This is usually set in `skyrim.ini` but you can only set diffuse/normal in the ini file. It is by default set to `dirt02.dds` and `dirt02_n.dds`, respectively. Since this plugin allows for more slots, you will need to define a "default landscape" texture set in a plugin. Just create a texture set and give it the editor ID `LandscapeDefault` and this plugin will automatically use it instead of skyrim.ini as the source.

## Technical Details

This plugin replaces the landscape shader material with a custom one that implements additional slots. As such, it is compatible with everything, but by its own will do nothing visually. It is up to ENB and/or CS to make use of the additional slots allocated to the shader.

These additional shader slots are defined as follows:

* `0` - `15` are the same as they were in vanilla
* `16` - `21` are either height maps or env mask maps, depending on which is available (if both are available, env mask map is used)
* `22` - `27` are for cubemaps/environment maps, which are only used if env mask is available

In addition, the following helper constants are defined:

* Slot `60`: `array` of 6x `uint32_t` which define the material for each texture.
  ```
  0 = no material
  1 = vanilla
  2 = parallax
  3 = env mapping / complex material
  ```

# Acknowledgements

This plugin would not be possible without the work of the Community Shaders team. Their PBR feature allows for terrain with PBR-relevant slots, so I was able to analyze their code to better understand a method of accomplishing this. The Community Shaders repository can be found [here](https://github.com/doodlum/skyrim-community-shaders). Give them a star!

Thanks to the helpful people on the Community Shaders discord, ENBSeries discord, and the Skyrim RE discord, for helping figure out some concepts needed for this plugin as well as testing.
