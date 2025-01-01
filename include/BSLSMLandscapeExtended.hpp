#pragma once

#include <array>

class BSLSMLandscapeExtended : public RE::BSLightingShaderMaterialBase
{
public:
	enum class TerrainShaderType {
		UNKNOWN,
		VANILLA,
		TERRAINPARALLAX,
		TERRAINENVMAP
	};

	inline static constexpr auto DiffuseTexture = static_cast<RE::BSTextureSet::Texture>(0);
	inline static constexpr auto NormalTexture = static_cast<RE::BSTextureSet::Texture>(1);
	inline static constexpr auto GlowTexture = static_cast<RE::BSTextureSet::Texture>(2);
	inline static constexpr auto HeightTexture = static_cast<RE::BSTextureSet::Texture>(3);
	inline static constexpr auto EnvTexture = static_cast<RE::BSTextureSet::Texture>(4);
	inline static constexpr auto EnvMaskTexture = static_cast<RE::BSTextureSet::Texture>(5);
	inline static constexpr auto SubsurfaceTexture = static_cast<RE::BSTextureSet::Texture>(6);
	inline static constexpr auto BacklightTexture = static_cast<RE::BSTextureSet::Texture>(7);

	BSLSMLandscapeExtended();
	~BSLSMLandscapeExtended();

	// override (BSLightingShaderMaterialBase)
	RE::BSShaderMaterial* Create() override;                                                                                      // 01
	void CopyMembers(RE::BSShaderMaterial* that) override;                                                                        // 02
	Feature GetFeature() const override;                                                                                          // 06
	void ClearTextures() override;                                                                                                // 09
	void ReceiveValuesFromRootMaterial(bool skinned, bool rimLighting, bool softLighting, bool backLighting, bool MSN) override;  // 0A
	uint32_t GetTextures(RE::NiSourceTexture** textures) override;                                                                // 0B

	// members
	std::uint32_t numLandscapeTextures = 0;

	// THESE ARE ORDERED IN A SPECIFIC WAY TO MATCH ASSERTIONS
	std::array<RE::NiPointer<RE::NiSourceTexture>, 6> landscapeDiffuseTex;

	std::array<uint32_t, 8> padding0D4;

	RE::NiPointer<RE::NiSourceTexture> terrainOverlayTexture;
	RE::NiPointer<RE::NiSourceTexture> terrainNoiseTexture;
	RE::NiColorA landBlendParams;

	std::array<RE::NiPointer<RE::NiSourceTexture>, 6> landscapeNormalTex;

	float terrainTexOffsetX = 0.f;
	float terrainTexOffsetY = 0.f;
	float terrainTexFade = 0.f;

	// Extended Texture Storage (After this line order doesn't matter)
	std::array<RE::NiPointer<RE::NiSourceTexture>, 6> landscapeGlowTex;
	std::array<RE::NiPointer<RE::NiSourceTexture>, 6> landscapeHeightTex;
	std::array<RE::NiPointer<RE::NiSourceTexture>, 6> landscapeEnvTex;
	std::array<RE::NiPointer<RE::NiSourceTexture>, 6> landscapeEnvMaskTex;
	std::array<RE::NiPointer<RE::NiSourceTexture>, 6> landscapeSubsurfaceTex;
	std::array<RE::NiPointer<RE::NiSourceTexture>, 6> landscapeBacklightTex;
};

static_assert(offsetof(BSLSMLandscapeExtended, numLandscapeTextures) == offsetof(RE::BSLightingShaderMaterialLandscape, numLandscapeTextures));
static_assert(offsetof(BSLSMLandscapeExtended, terrainOverlayTexture) == offsetof(RE::BSLightingShaderMaterialLandscape, terrainOverlayTexture));
static_assert(offsetof(BSLSMLandscapeExtended, terrainNoiseTexture) == offsetof(RE::BSLightingShaderMaterialLandscape, terrainNoiseTexture));
static_assert(offsetof(BSLSMLandscapeExtended, landBlendParams) == offsetof(RE::BSLightingShaderMaterialLandscape, landBlendParams));
static_assert(offsetof(BSLSMLandscapeExtended, terrainTexOffsetX) == offsetof(RE::BSLightingShaderMaterialLandscape, terrainTexOffsetX));
static_assert(offsetof(BSLSMLandscapeExtended, terrainTexOffsetY) == offsetof(RE::BSLightingShaderMaterialLandscape, terrainTexOffsetY));
static_assert(offsetof(BSLSMLandscapeExtended, terrainTexFade) == offsetof(RE::BSLightingShaderMaterialLandscape, terrainTexFade));
