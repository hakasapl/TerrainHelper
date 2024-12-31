#include <RE/B/BGSCameraPath.h>
#include <RE/B/BSFixedString.h>
#include <RE/B/BSShaderManager.h>
#include <RE/B/BSShader.h>
#include <RE/I/INISettingCollection.h>
#include <RE/Offsets_RTTI.h>
#include <RE/Offsets_VTABLE.h>
#include <RE/Skyrim.h>
#include <REL/Relocation.h>
#include <SKSE/API.h>
#include <SKSE/Events.h>
#include <SKSE/Logger.h>
#include <SKSE/SKSE.h>
#include <SKSE/Trampoline.h>
#include <spdlog/sinks/basic_file_sink.h>

#include "PCH.h"
#include "BSLSMLandscapeExtended.hpp"

#define DLLEXPORT __declspec(dllexport)

using namespace std;

void SetupLog() {
  auto logsFolder = SKSE::log::log_directory();
  if (!logsFolder) {
    SKSE::stl::report_and_fail(
      "SKSE log_directory not provided, logs disabled.");
  }
  auto logFilePath = *logsFolder / "skyrimterrainslotunlocker.log";
  auto fileLoggerPtr = std::make_shared<spdlog::sinks::basic_file_sink_mt>(
    logFilePath.string(), true);
  auto loggerPtr =
    std::make_shared<spdlog::logger>("log", std::move(fileLoggerPtr));
  spdlog::set_default_logger(std::move(loggerPtr));
  spdlog::set_level(spdlog::level::trace);
  spdlog::flush_on(spdlog::level::trace);
}

struct BSLightingShader_SetupMaterial
{
  static void thunk(RE::BSLightingShader* shader, RE::BSLightingShaderMaterialBase const* material)
  {
    const auto lightingType = (shader->currentRawTechnique >> 24) & 0x3F;
    if (lightingType != 8 && lightingType != 19) {
      // Not terrain
      return func(shader, material);
    }

    // get renderer and shadow state
    auto shadowState = RE::BSGraphics::RendererShadowState::GetSingleton();
    auto renderer = RE::BSGraphics::Renderer::GetSingleton();

    // get material
    auto materialBase = static_cast<const BSLSMLandscapeExtended*>(material);

    static constexpr size_t NormalStartIndex = 7;
    static constexpr size_t ExtendedStartIndex = 16;

    // Assign textures to slots
    if (materialBase->terrainOverlayTexture != nullptr) {
      shadowState->SetPSTexture(13, materialBase->terrainOverlayTexture->rendererTexture);
      shadowState->SetPSTextureAddressMode(13, RE::BSGraphics::TextureAddressMode::kClampSClampT);
      shadowState->SetPSTextureFilterMode(13, RE::BSGraphics::TextureFilterMode::kAnisotropic);
    }

    if (materialBase->terrainNoiseTexture != nullptr) {
      shadowState->SetPSTexture(15, materialBase->terrainNoiseTexture->rendererTexture);
      shadowState->SetPSTextureAddressMode(15, RE::BSGraphics::TextureAddressMode::kWrapSWrapT);
      shadowState->SetPSTextureFilterMode(15, RE::BSGraphics::TextureFilterMode::kBilinear);
    }

    for (uint32_t textureI = 0; textureI < 6; ++textureI) {
      // Diffuse
      if (materialBase->landscapeDiffuseTex[textureI] != nullptr) {
        shadowState->SetPSTexture(textureI, materialBase->landscapeDiffuseTex[textureI]->rendererTexture);
        shadowState->SetPSTextureAddressMode(textureI, RE::BSGraphics::TextureAddressMode::kWrapSWrapT);
        shadowState->SetPSTextureFilterMode(textureI, RE::BSGraphics::TextureFilterMode::kAnisotropic);
      }
      // Normal
      if (materialBase->landscapeNormalTex[textureI] != nullptr) {
        const uint32_t normalIndex = NormalStartIndex + textureI;
        shadowState->SetPSTexture(NormalStartIndex, materialBase->landscapeNormalTex[textureI]->rendererTexture);
        shadowState->SetPSTextureAddressMode(NormalStartIndex, RE::BSGraphics::TextureAddressMode::kWrapSWrapT);
        shadowState->SetPSTextureFilterMode(NormalStartIndex, RE::BSGraphics::TextureFilterMode::kAnisotropic);
      }
      // Glow
      if (materialBase->landscapeGlowTex[textureI] != nullptr) {
        shadowState->SetPSTexture(ExtendedStartIndex, materialBase->landscapeGlowTex[textureI]->rendererTexture);
        shadowState->SetPSTextureAddressMode(ExtendedStartIndex, RE::BSGraphics::TextureAddressMode::kWrapSWrapT);
        shadowState->SetPSTextureFilterMode(ExtendedStartIndex, RE::BSGraphics::TextureFilterMode::kAnisotropic);
      }
      // Height
      if (materialBase->landscapeHeightTex[textureI] != nullptr) {
        const uint32_t heightIndex = ExtendedStartIndex + 6;
        shadowState->SetPSTexture(heightIndex, materialBase->landscapeHeightTex[textureI]->rendererTexture);
        shadowState->SetPSTextureAddressMode(heightIndex, RE::BSGraphics::TextureAddressMode::kWrapSWrapT);
        shadowState->SetPSTextureFilterMode(heightIndex, RE::BSGraphics::TextureFilterMode::kAnisotropic);
      }
      // Environment
      if (materialBase->landscapeEnvTex[textureI] != nullptr) {
        const uint32_t envIndex = ExtendedStartIndex + 12;
        shadowState->SetPSTexture(envIndex, materialBase->landscapeEnvTex[textureI]->rendererTexture);
        shadowState->SetPSTextureAddressMode(envIndex, RE::BSGraphics::TextureAddressMode::kWrapSWrapT);
        shadowState->SetPSTextureFilterMode(envIndex, RE::BSGraphics::TextureFilterMode::kAnisotropic);
      }
      // Environment Mask
      if (materialBase->landscapeEnvMaskTex[textureI] != nullptr) {
        const uint32_t envMaskIndex = ExtendedStartIndex + 18;
        shadowState->SetPSTexture(envMaskIndex, materialBase->landscapeEnvMaskTex[textureI]->rendererTexture);
        shadowState->SetPSTextureAddressMode(envMaskIndex, RE::BSGraphics::TextureAddressMode::kWrapSWrapT);
        shadowState->SetPSTextureFilterMode(envMaskIndex, RE::BSGraphics::TextureFilterMode::kAnisotropic);
      }
      // Subsurface
      if (materialBase->landscapeSubsurfaceTex[textureI] != nullptr) {
        const uint32_t subsurfaceIndex = ExtendedStartIndex + 24;
        shadowState->SetPSTexture(subsurfaceIndex, materialBase->landscapeSubsurfaceTex[textureI]->rendererTexture);
        shadowState->SetPSTextureAddressMode(subsurfaceIndex, RE::BSGraphics::TextureAddressMode::kWrapSWrapT);
        shadowState->SetPSTextureFilterMode(subsurfaceIndex, RE::BSGraphics::TextureFilterMode::kAnisotropic);
      }
      // Backlight
      if (materialBase->landscapeBacklightTex[textureI] != nullptr) {
        const uint32_t backlightIndex = ExtendedStartIndex + 30;
        shadowState->SetPSTexture(backlightIndex, materialBase->landscapeBacklightTex[textureI]->rendererTexture);
        shadowState->SetPSTextureAddressMode(backlightIndex, RE::BSGraphics::TextureAddressMode::kWrapSWrapT);
        shadowState->SetPSTextureFilterMode(backlightIndex, RE::BSGraphics::TextureFilterMode::kAnisotropic);
      }
    }
  };

  static inline REL::Relocation<decltype(thunk)> func;
};

RE::TESLandTexture* GetDefaultLandTexture()
{
  static const auto defaultLandTextureAddress = REL::Relocation<RE::TESLandTexture**>(RELOCATION_ID(514783, 400936));
  return *defaultLandTextureAddress;
}

struct TESObjectLAND_SetupMaterial
{
  static bool thunk(RE::TESObjectLAND* land)
  {
    if (land->loadedData == nullptr || land->loadedData->mesh[0] == nullptr) {
      return false;
    }

    static const auto settings = RE::INISettingCollection::GetSingleton();
    static const bool bEnableLandFade = settings->GetSetting("bEnableLandFade:Display");
    static const bool bDrawLandShadows = settings->GetSetting("bDrawLandShadows:Display");

    for (uint32_t quadI = 0; quadI < 4; ++quadI) {
      auto shaderProperty = static_cast<RE::BSLightingShaderProperty*>(RE::MemoryManager::GetSingleton()->Allocate(REL::Module::IsVR() ? 0x178 : sizeof(RE::BSLightingShaderProperty), 0, false));
      shaderProperty->Ctor();

      // Create a new material
      BSLSMLandscapeExtended srcMaterial;
      shaderProperty->LinkMaterial(&srcMaterial, true);

      // Get material after assigning it
      auto material = static_cast<BSLSMLandscapeExtended*>(shaderProperty->material);
      const auto& stateData = RE::BSGraphics::State::GetSingleton()->GetRuntimeData();

      // Set initial textures
      for (uint32_t textureIndex = 0; textureIndex < 6; ++textureIndex) {
        material->landscapeDiffuseTex[textureIndex] = stateData.defaultTextureBlack;
        material->landscapeNormalTex[textureIndex] = stateData.defaultTextureNormalMap;
        material->landscapeGlowTex[textureIndex] = stateData.defaultTextureBlack;
        material->landscapeHeightTex[textureIndex] = stateData.defaultTextureBlack;
        material->landscapeEnvTex[textureIndex] = stateData.defaultTextureBlack;
        material->landscapeEnvMaskTex[textureIndex] = stateData.defaultTextureBlack;
        material->landscapeSubsurfaceTex[textureIndex] = stateData.defaultTextureBlack;
        material->landscapeBacklightTex[textureIndex] = stateData.defaultTextureBlack;
      }

      // Create array of texture sets (6 tiles)
      std::array<RE::TESLandTexture*, 6> textureSets;
      if (auto defTexture = land->loadedData->defQuadTextures[quadI]) {
        textureSets[0] = defTexture;
      }
      else {
        textureSets[0] = GetDefaultLandTexture();
      }
      for (uint32_t textureI = 1; textureI < 6; ++textureI) {
        if (auto landTexture = land->loadedData->quadTextures[quadI][textureI]) {
          textureSets[textureI] = landTexture;
        }
      }

      // Assign textures to material
      for (uint32_t textureI = 0; textureI < 6; ++textureI) {
        auto& txSet = textureSets[textureI]->textureSet;
        if (txSet == nullptr) {
          continue;
        }

        txSet->SetTexture(RE::BSTextureSet::Texture::kDiffuse, material->landscapeDiffuseTex[textureI]);
        txSet->SetTexture(RE::BSTextureSet::Texture::kNormal, material->landscapeNormalTex[textureI]);
        txSet->SetTexture(RE::BSTextureSet::Texture::kGlowMap, material->landscapeGlowTex[textureI]);
        txSet->SetTexture(RE::BSTextureSet::Texture::kHeight, material->landscapeHeightTex[textureI]);
        txSet->SetTexture(RE::BSTextureSet::Texture::kEnvironment, material->landscapeEnvTex[textureI]);
        txSet->SetTexture(RE::BSTextureSet::Texture::kEnvironmentMask, material->landscapeEnvMaskTex[textureI]);
        txSet->SetTexture(RE::BSTextureSet::Texture::kMultilayer, material->landscapeSubsurfaceTex[textureI]);
        txSet->SetTexture(RE::BSTextureSet::Texture::kBacklightMask, material->landscapeBacklightTex[textureI]);
      }

      // Other properties
      if (bEnableLandFade) {
        shaderProperty->unk108 = false;
      }

      bool noLODLandBlend = false;
      auto tes = RE::TES::GetSingleton();
      auto worldSpace = tes->GetRuntimeData2().worldSpace;
      if (worldSpace != nullptr) {
        if (auto terrainManager = worldSpace->GetTerrainManager()) {
          noLODLandBlend = reinterpret_cast<bool*>(terrainManager)[0x36];
        }
      }
      shaderProperty->SetFlags(RE::BSShaderProperty::EShaderPropertyFlag8::kMultiTextureLandscape, true);
      shaderProperty->SetFlags(RE::BSShaderProperty::EShaderPropertyFlag8::kReceiveShadows, true);
      shaderProperty->SetFlags(RE::BSShaderProperty::EShaderPropertyFlag8::kCastShadows, bDrawLandShadows);
      shaderProperty->SetFlags(RE::BSShaderProperty::EShaderPropertyFlag8::kNoLODLandBlend, noLODLandBlend);

      shaderProperty->SetFlags(RE::BSShaderProperty::EShaderPropertyFlag8::kVertexLighting, true);

      // Geometry setup
      const auto& children = land->loadedData->mesh[quadI]->GetChildren();
      auto geometry = children.empty() ? nullptr : static_cast<RE::BSGeometry*>(children[0].get());
      shaderProperty->SetupGeometry(geometry);
      if (geometry != nullptr) {
        geometry->GetGeometryRuntimeData().properties[1] = RE::NiPointer(shaderProperty);
      }

      RE::BSShaderManager::State::GetSingleton().shadowSceneNode[0]->AttachObject(geometry);
    }

    return true;
  }

  static inline REL::Relocation<decltype(thunk)> func;
};

extern "C" DLLEXPORT bool SKSEAPI SKSEPlugin_Load(const SKSE::LoadInterface* skse) {
  SKSE::Init(skse);
  SetupLog();
  spdlog::info("Plugin loaded.");

  // Allocate Trampoline
  SKSE::AllocTrampoline(64);

  // Hooks
  stl::write_vfunc<0x4, BSLightingShader_SetupMaterial>(RE::VTABLE_BSLightingShader[0]);
  stl::detour_thunk<TESObjectLAND_SetupMaterial>(REL::RelocationID(18368, 18791));

  return true;
}

extern "C" DLLEXPORT constinit auto SKSEPlugin_Version = []() noexcept {
  SKSE::PluginVersionData v;
  v.PluginName("SkyrimTerrainSlotUnlocker");
  v.PluginVersion(REL::Version("0.1.0"));
  v.UsesAddressLibrary();
  v.UsesNoStructs();
  return v;
}();

extern "C" DLLEXPORT bool SKSEAPI SKSEPlugin_Query(const SKSE::QueryInterface*, SKSE::PluginInfo* pluginInfo)
{
  pluginInfo->name = SKSEPlugin_Version.pluginName;
  pluginInfo->infoVersion = SKSE::PluginInfo::kVersion;
  pluginInfo->version = SKSEPlugin_Version.pluginVersion;
  return true;
}
