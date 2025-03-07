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

#include <unordered_set>

#include "CLIBUtil/editorID.hpp"

#include "PCH.h"

#define DLLEXPORT __declspec(dllexport)

using namespace std;

unordered_set<string> texturesErrorLogged;
void SetupLog() {
    auto logsFolder = SKSE::log::log_directory();
    if (!logsFolder) {
        SKSE::stl::report_and_fail("SKSE log_directory not provided, logs disabled.");
    }
    auto logFilePath = *logsFolder / "TerrainHelper.log";
    auto fileLoggerPtr = std::make_shared<spdlog::sinks::basic_file_sink_mt>(logFilePath.string(), true);
    auto loggerPtr = std::make_shared<spdlog::logger>("log", std::move(fileLoggerPtr));
    spdlog::set_default_logger(std::move(loggerPtr));
    spdlog::set_level(spdlog::level::trace);
    spdlog::flush_on(spdlog::level::trace);
}

struct ExtendedSlots {
    array<RE::NiSourceTexturePtr, 6> parallax;
    array<RE::NiSourceTexturePtr, 6> envmask;
};
unordered_map<uint32_t, ExtendedSlots> extendedSlots;

struct BSLightingShader_SetupMaterial
{
    static void thunk(RE::BSLightingShader* shader, RE::BSLightingShaderMaterialBase const* material)
    {
        func(shader, material);

        if (!extendedSlots.contains(material->hashKey)) {
            // hash does not exists
            return;
        }

        const auto materialBase = extendedSlots[material->hashKey];

        // get renderer and shadow state
        auto shadowState = RE::BSGraphics::RendererShadowState::GetSingleton();

        const auto& stateData = RE::BSGraphics::State::GetSingleton()->GetRuntimeData();

        static constexpr size_t ParallaxStartIndex = 16;  // 16-21
        static constexpr size_t EnvmaskStartIndex = 22;  // 22-27

        // Populate extended slots
        for (uint32_t textureI = 0; textureI < 6; ++textureI) {
            const uint32_t heightIndex = ParallaxStartIndex + textureI;
            if (materialBase.parallax[textureI] != nullptr && materialBase.parallax[textureI] != stateData.defaultTextureNormalMap) {
                shadowState->SetPSTexture(heightIndex, materialBase.parallax[textureI]->rendererTexture);
            }
            else {
                // set default texture
                shadowState->SetPSTexture(heightIndex, nullptr);
            }

            // ENV MASK DISABLED FOR NOW
            //if (materialBase.envmask[textureI] != nullptr) {
            //    const uint32_t heightIndex = EnvmaskStartIndex + textureI;
            //    shadowState->SetPSTexture(heightIndex, materialBase.envmask[textureI]->rendererTexture);
            //}
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
        bool vanResult = func(land);

        if (land->loadedData == nullptr || land->loadedData->mesh[0] == nullptr || !vanResult) {
            // this is not terrain or vanilla material failed
            return vanResult;
        }

        const auto& stateData = RE::BSGraphics::State::GetSingleton()->GetRuntimeData();

        for (uint32_t quadI = 0; quadI < 4; ++quadI) {
            // Get hash key of vanilla material
            uint32_t hashKey = 0;

            const auto& children = land->loadedData->mesh[quadI]->GetChildren();
            auto geometry = children.empty() ? nullptr : static_cast<RE::BSGeometry*>(children[0].get());
            if (geometry != nullptr) {
                const auto shaderProp = static_cast<RE::BSLightingShaderProperty*>(geometry->GetGeometryRuntimeData().properties[1].get());
                if (shaderProp != nullptr) {
                    hashKey = shaderProp->GetBaseMaterial()->hashKey;
                }
            }

            if (hashKey == 0) {
                // continue if cannot find hash key
                continue;
            }

            if (!extendedSlots.contains(hashKey)) {
                extendedSlots[hashKey] = {};
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
                textureSets[textureI + 1] = land->loadedData->quadTextures[quadI][textureI];
            }

            // Assign textures to material
            for (uint32_t textureI = 0; textureI < 6; ++textureI) {
                if (textureSets[textureI] == nullptr || textureSets[textureI]->textureSet == nullptr) {
                    continue;
                }

                auto txSet = textureSets[textureI]->textureSet;

                if (txSet->GetTexturePath(static_cast<RE::BSTextureSet::Texture>(3)) != nullptr) {
                    txSet->SetTexture(static_cast<RE::BSTextureSet::Texture>(3), extendedSlots[hashKey].parallax[textureI]);
                    if (extendedSlots[hashKey].parallax[textureI] == stateData.defaultTextureNormalMap) {
                        // this file was not found
                        if (get<1>(texturesErrorLogged.insert(txSet->GetTexturePath(static_cast<RE::BSTextureSet::Texture>(3))))) {
                            spdlog::error("Unable to find parallax map {} while setting up material", txSet->GetTexturePath(static_cast<RE::BSTextureSet::Texture>(3)));
                        }
                    }
                }

                // ENV MASK DISABLED FOR NOW
                //if (txSet->GetTexturePath(static_cast<RE::BSTextureSet::Texture>(5)) != nullptr) {
                //    txSet->SetTexture(static_cast<RE::BSTextureSet::Texture>(5), extendedSlots[hashKey].envmask[textureI]);
                //}
            }
        }

        return true;
    }

    static inline REL::Relocation<decltype(thunk)> func;
};

void onDataLoaded() {
    // Replace default texture set if needed
    const auto defaultLandTextureSet = RE::TESForm::LookupByEditorID<RE::BGSTextureSet>("LandscapeDefault");
    if (defaultLandTextureSet != nullptr) {
        if (auto* defaultLandTexture = GetDefaultLandTexture()) {
            spdlog::info("Replacing default land texture set record with {}", clib_util::editorID::get_editorID(defaultLandTextureSet));
            defaultLandTexture->textureSet = defaultLandTextureSet;
        }
        else {
            spdlog::error("Default land texture set record not found");
        }
    }
    else {
        spdlog::info("LandscapeDefault EDID texture set not found, using default");
    }
}

void MessageHandler(SKSE::MessagingInterface::Message* a_msg) {
    switch (a_msg->type) {
    case SKSE::MessagingInterface::kDataLoaded:
        onDataLoaded();
        break;
    }
}

extern "C" DLLEXPORT bool SKSEAPI SKSEPlugin_Load(const SKSE::LoadInterface* skse) {
    SKSE::Init(skse);
    SetupLog();
    spdlog::info("{} version {} loaded", PLUGIN_NAME, PLUGIN_VERSION);

    // Register messaging interface
    auto messaging = SKSE::GetMessagingInterface();
    if (!messaging->RegisterListener("SKSE", MessageHandler)) {
        return false;
    }

    // Allocate Trampoline
    SKSE::AllocTrampoline(64);

    // Hooks
    stl::write_vfunc<0x4, BSLightingShader_SetupMaterial>(RE::VTABLE_BSLightingShader[0]);
    stl::detour_thunk<TESObjectLAND_SetupMaterial>(REL::RelocationID(18368, 18791));

    return true;
}

extern "C" DLLEXPORT constinit auto SKSEPlugin_Version = []() noexcept {
    SKSE::PluginVersionData v;
    v.PluginName(PLUGIN_NAME);
    v.PluginVersion(REL::Version(PLUGIN_VERSION));
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
