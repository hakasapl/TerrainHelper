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
    func(shader, material);

    const auto lightingType = (shader->currentRawTechnique >> 24) & 0x3F;
    if (lightingType != 8 && lightingType != 19) {
      // Not terrain
      return;
    }

    //auto context = RE::BSGraphics::Renderer::GetSingleton()->GetRuntimeData().context;
    auto shadowState = RE::BSGraphics::RendererShadowState::GetSingleton();

    // Get each possible texture from the material
    auto* materialEnvMapping = static_cast<const RE::BSLightingShaderMaterialEnvmap*>(material);
    const auto& envMaskTex = materialEnvMapping->envMaskTexture;
    const auto& envTex = materialEnvMapping->envTexture;

    auto* materialGlow = static_cast<const RE::BSLightingShaderMaterialGlowmap*>(material);
    const auto& glowTex = materialGlow->glowTexture;

    auto* materialParallax = static_cast<const RE::BSLightingShaderMaterialParallax*>(material);
    const auto& heightTex = materialParallax->heightTexture;

    auto* materialMultilayerParallax = static_cast<const RE::BSLightingShaderMaterialMultiLayerParallax*>(material);
    const auto& layerTex = materialMultilayerParallax->layerTexture;

    const auto& backlightTex = material->specularBackLightingTexture;

    // Check if any non-normal texture is defined and get it from relevant shader
    if (envMaskTex != nullptr) {
      // Env mask in slot 16
      shadowState->SetPSTexture(16, envMaskTex->rendererTexture);
    }

    if (glowTex != nullptr) {
      // Glow map in slot 17
      shadowState->SetPSTexture(17, glowTex->rendererTexture);
    }

    if (heightTex != nullptr) {
      // Height map in slot 18
      shadowState->SetPSTexture(18, heightTex->rendererTexture);
    }

    if (envTex != nullptr) {
      // Env map in slot 19
      shadowState->SetPSTexture(19, envTex->rendererTexture);
    }

    if (layerTex != nullptr) {
      // Multilayer in slot 20
      shadowState->SetPSTexture(20, layerTex->rendererTexture);
    }

    if (backlightTex != nullptr) {
      // Backlight in slot 21
      shadowState->SetPSTexture(21, backlightTex->rendererTexture);
    }
  };

  static inline REL::Relocation<decltype(thunk)> func;
};

struct TESObjectLAND_SetupMaterial
{
  static bool thunk(RE::TESObjectLAND* land)
  {
    return func(land);
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
