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

#include "CLIBUtil/editorID.hpp"

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
    // get material
    auto materialBase = dynamic_cast<const BSLSMLandscapeExtended*>(material);
    if (materialBase == nullptr) {
      // Not our material
      return func(shader, material);
    }

    // get renderer and shadow state
    auto shadowState = RE::BSGraphics::RendererShadowState::GetSingleton();
    auto runtimeState = RE::BSGraphics::State::GetSingleton()->GetRuntimeData();

    static constexpr size_t NormalStartIndex = 7;
    static constexpr size_t ExtendedStartIndex = 16;

    for (uint32_t textureI = 0; textureI < 6; ++textureI) {
      // Diffuse
      if (materialBase->landscapeDiffuseTex[textureI] != nullptr && materialBase->landscapeDiffuseTex[textureI] != runtimeState.defaultTextureBlack) {
        shadowState->SetPSTexture(textureI, materialBase->landscapeDiffuseTex[textureI]->rendererTexture);
        shadowState->SetPSTextureAddressMode(textureI, RE::BSGraphics::TextureAddressMode::kWrapSWrapT);
        shadowState->SetPSTextureFilterMode(textureI, RE::BSGraphics::TextureFilterMode::kAnisotropic);
      }
      // Normal
      if (materialBase->landscapeNormalTex[textureI] != nullptr && materialBase->landscapeNormalTex[textureI] != runtimeState.defaultTextureNormalMap) {
        const uint32_t normalIndex = NormalStartIndex + textureI;
        shadowState->SetPSTexture(normalIndex, materialBase->landscapeNormalTex[textureI]->rendererTexture);
        shadowState->SetPSTextureAddressMode(NormalStartIndex, RE::BSGraphics::TextureAddressMode::kWrapSWrapT);
        shadowState->SetPSTextureFilterMode(NormalStartIndex, RE::BSGraphics::TextureFilterMode::kAnisotropic);
      }

      // Extended Slots
      /*if (materialBase->landscapeEnvMaskTex[textureI] != nullptr &&
          materialBase->landscapeEnvMaskTex[textureI] != runtimeState.defaultTextureBlack &&
          materialBase->landscapeEnvMaskTex[textureI] != runtimeState.defaultTextureNormalMap) {
        // Has an Environment Mask
        const uint32_t envMaskIndex = ExtendedStartIndex + textureI;
        shadowState->SetPSTexture(envMaskIndex, materialBase->landscapeEnvMaskTex[textureI]->rendererTexture);

        // Check if Cubemap exists
        if (materialBase->landscapeEnvTex[textureI] != nullptr && materialBase->landscapeEnvTex[textureI] != runtimeState.defaultReflectionCubeMap && materialBase->landscapeEnvTex[textureI] != runtimeState.defaultTextureNormalMap) {
          const uint32_t envIndex = ExtendedStartIndex + 6 + textureI;
          shadowState->SetPSTexture(envIndex, materialBase->landscapeEnvTex[textureI]->rendererTexture);
        }
      }
      else if (materialBase->landscapeHeightTex[textureI] != nullptr &&
               materialBase->landscapeHeightTex[textureI] != runtimeState.defaultHeightMap &&
               materialBase->landscapeHeightTex[textureI] != runtimeState.defaultTextureNormalMap) {
        // Has a height map
        const uint32_t heightIndex = ExtendedStartIndex + textureI;
        shadowState->SetPSTexture(heightIndex, materialBase->landscapeHeightTex[textureI]->rendererTexture);
      }*/
    }

    // Assign terrain overlay and noise texture to slots
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
  };

  static inline REL::Relocation<decltype(thunk)> func;
};

RE::TESLandTexture* GetDefaultLandTexture()
{
  // Find textureset by editorid
  auto dataHandler = RE::TESDataHandler::GetSingleton();
  if (!dataHandler) {
    return nullptr;
  }

  // Get all TextureSet forms
  const auto& textureSets = dataHandler->GetFormArray<RE::TESLandTexture>();
  for (auto* textureSet : textureSets) {
    if (textureSet != nullptr && clib_util::editorID::get_editorID(textureSet) == "LDefault") {
      return textureSet;
    }
  }

  // Get it from vanilla address
  static const auto defaultLandTextureAddress = REL::Relocation<RE::TESLandTexture**>(RELOCATION_ID(514783, 400936));
  return *defaultLandTextureAddress;
}

struct TESObjectLAND_SetupMaterial
{
  static bool thunk(RE::TESObjectLAND* land)
  {
    if (land->loadedData == nullptr || land->loadedData->mesh[0] == nullptr) {
      return func(land);
    }

    // Check if terrain has PBR in it (preserves compatibility with community shaders, since they do their own PBR terrain)
    for (uint32_t quadI = 0; quadI < 4; ++quadI) {
      for (uint32_t textureI = 0; textureI < 6; ++textureI) {
        if (land->loadedData->quadTextures[quadI][textureI] != nullptr && land->loadedData->quadTextures[quadI][textureI]->textureSet != nullptr) {
          // Get DiffusePath
          const auto EditorID = clib_util::editorID::get_editorID(land->loadedData->quadTextures[quadI][textureI]->textureSet);
          // Build full path
          const auto JSONPath = "PBRTextureSets\\" + EditorID + ".json";
          // Check if file exists
          RE::BSResourceNiBinaryStream fileStream{ JSONPath };
          if (fileStream.good()) {
            // File exists, this is PBR terrain, leave it to CS
            return false;
          }
        }
      }
    }

    static const auto settings = RE::INISettingCollection::GetSingleton();
    static const bool bEnableLandFade = settings->GetSetting("bEnableLandFade:Display");
    static const bool bDrawLandShadows = settings->GetSetting("bDrawLandShadows:Display");

    for (uint32_t quadI = 0; quadI < 4; ++quadI) {
      auto shaderProperty = static_cast<RE::BSLightingShaderProperty*>(RE::MemoryManager::GetSingleton()->Allocate(REL::Module::IsVR() ? 0x178 : sizeof(RE::BSLightingShaderProperty), 0, false));
      shaderProperty->Ctor();

      // Create a new material
      {
        BSLSMLandscapeExtended material;
        shaderProperty->LinkMaterial(&material, true);
      }

      // Get material after assigning it
      auto material = static_cast<BSLSMLandscapeExtended*>(shaderProperty->material);
      const auto& stateData = RE::BSGraphics::State::GetSingleton()->GetRuntimeData();

      // Set initial textures
      for (uint32_t textureI = 0; textureI < 6; ++textureI) {
        material->landscapeDiffuseTex[textureI] = stateData.defaultTextureBlack;
        material->landscapeNormalTex[textureI] = stateData.defaultTextureNormalMap;
        material->landscapeGlowTex[textureI] = stateData.defaultTextureBlack;
        material->landscapeHeightTex[textureI] = stateData.defaultHeightMap;
        material->landscapeEnvTex[textureI] = stateData.defaultReflectionCubeMap;
        material->landscapeEnvMaskTex[textureI] = stateData.defaultTextureBlack;
        material->landscapeSubsurfaceTex[textureI] = stateData.defaultTextureBlack;
        material->landscapeBacklightTex[textureI] = stateData.defaultTextureBlack;
      }

      // Create array of texture sets (6 tiles)
      std::array<RE::TESLandTexture*, 6> textureSets;
      if (auto defTexture = land->loadedData->defQuadTextures[quadI]) {
        textureSets[0] = defTexture;
      }
      else {
        textureSets[0] = GetDefaultLandTexture();
      }
      for (uint32_t textureI = 0; textureI < 5; ++textureI) {
        if (auto landTexture = land->loadedData->quadTextures[quadI][textureI]) {
          textureSets[textureI + 1] = landTexture;
        }
        else {
          textureSets[textureI + 1] = nullptr;
        }
      }

      // Assign textures to material
      for (uint32_t textureI = 0; textureI < 6; ++textureI) {
        if (textureSets[textureI] == nullptr || textureSets[textureI]->textureSet == nullptr) {
          continue;
        }

        auto txSet = textureSets[textureI]->textureSet;
        if (txSet->GetTexturePath(BSLSMLandscapeExtended::DiffuseTexture) != nullptr) {
          txSet->SetTexture(BSLSMLandscapeExtended::DiffuseTexture, material->landscapeDiffuseTex[textureI]);
        }
        if (txSet->GetTexturePath(BSLSMLandscapeExtended::NormalTexture) != nullptr) {
          txSet->SetTexture(BSLSMLandscapeExtended::NormalTexture, material->landscapeNormalTex[textureI]);
        }
        if (txSet->GetTexturePath(BSLSMLandscapeExtended::GlowTexture) != nullptr) {
          txSet->SetTexture(BSLSMLandscapeExtended::GlowTexture, material->landscapeGlowTex[textureI]);
        }
        if (txSet->GetTexturePath(BSLSMLandscapeExtended::HeightTexture) != nullptr) {
          txSet->SetTexture(BSLSMLandscapeExtended::HeightTexture, material->landscapeHeightTex[textureI]);
        }
        if (txSet->GetTexturePath(BSLSMLandscapeExtended::EnvTexture) != nullptr) {
          txSet->SetTexture(BSLSMLandscapeExtended::EnvTexture, material->landscapeEnvTex[textureI]);
        }
        if (txSet->GetTexturePath(BSLSMLandscapeExtended::EnvMaskTexture) != nullptr) {
          txSet->SetTexture(BSLSMLandscapeExtended::EnvMaskTexture, material->landscapeEnvMaskTex[textureI]);
        }
        if (txSet->GetTexturePath(BSLSMLandscapeExtended::SubsurfaceTexture) != nullptr) {
          txSet->SetTexture(BSLSMLandscapeExtended::SubsurfaceTexture, material->landscapeSubsurfaceTex[textureI]);
        }
        if (txSet->GetTexturePath(BSLSMLandscapeExtended::BacklightTexture) != nullptr) {
          txSet->SetTexture(BSLSMLandscapeExtended::BacklightTexture, material->landscapeBacklightTex[textureI]);
        }

        // Update numlandscape textures
        material->numLandscapeTextures = std::max(material->numLandscapeTextures, textureI + 1);
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
