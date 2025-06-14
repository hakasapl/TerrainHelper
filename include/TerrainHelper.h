#pragma once

#include <array>
#include <shared_mutex>
#include <string>
#include <unordered_map>
#include <unordered_set>

class TerrainHelper {
private:
	struct ExtendedSlots {
		std::array<RE::NiSourceTexturePtr, 6> parallax;
	};

	static std::shared_mutex extendedSlotsMutex;
	static std::unordered_map<uint32_t, ExtendedSlots> extendedSlots;

	static std::unordered_set<std::string> texturesErrorLogged;
	static RE::BGSTextureSet* defaultLandTexture;

	static bool enabled;
	static bool defaultSetReplaced;

	[[nodiscard]] static RE::BGSTextureSet* GetSeasonalSwap(RE::BGSTextureSet* textureSet);

public:
	static void TESObjectLAND_SetupMaterial(RE::TESObjectLAND* land);
	static void BSLightingShader_SetupMaterial(RE::BSLightingShader* shader, RE::BSLightingShaderMaterialBase const* material);
	static void ReplaceDefaultLandscapeSet();
};