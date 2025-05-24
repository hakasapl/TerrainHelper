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

#include "TerrainHelper.h"

#define DLLEXPORT __declspec(dllexport)

using namespace std;


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

struct BSLightingShader_SetupMaterial
{
    static void thunk(RE::BSLightingShader* shader, RE::BSLightingShaderMaterialBase const* material)
    {
        func(shader, material);

        TerrainHelper::BSLightingShader_SetupMaterial(shader, material);
    };

    static inline REL::Relocation<decltype(thunk)> func;
};

struct TESObjectLAND_SetupMaterial
{
    static bool thunk(RE::TESObjectLAND* land)
    {
        bool vanResult = func(land);

        if (!vanResult || land == nullptr || land->loadedData == nullptr || land->loadedData->mesh[0] == nullptr) {
            // this is not terrain or vanilla material failed
            return vanResult;
        }

        TerrainHelper::TESObjectLAND_SetupMaterial(land);
        return true;
    }

    static inline REL::Relocation<decltype(thunk)> func;
};

struct SetPerFrameBuffers
{
    static void thunk(void* renderer)
    {
		func(renderer);
        TerrainHelper::ReplaceDefaultLandscapeSet();
    }
    static inline REL::Relocation<decltype(thunk)> func;
};

void MessageHandler(SKSE::MessagingInterface::Message* a_msg) {
    switch (a_msg->type) {
    case SKSE::MessagingInterface::kDataLoaded:
        TerrainHelper::ReplaceDefaultLandscapeSet();
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

    // Hooks
	spdlog::info("Hooking functions");
    stl::write_vfunc<0x4, BSLightingShader_SetupMaterial>(RE::VTABLE_BSLightingShader[0]);
    stl::detour_thunk<TESObjectLAND_SetupMaterial>(REL::RelocationID(18368, 18791));
    stl::detour_thunk<SetPerFrameBuffers>(REL::RelocationID(75570, 77371));

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

extern "C" DLLEXPORT bool SKSEAPI SKSEPlugin_Query(const SKSE::QueryInterface*, SKSE::PluginInfo* pluginInfo) {
    pluginInfo->name = SKSEPlugin_Version.pluginName;
    pluginInfo->infoVersion = SKSE::PluginInfo::kVersion;
    pluginInfo->version = SKSEPlugin_Version.pluginVersion;
    return true;
}
