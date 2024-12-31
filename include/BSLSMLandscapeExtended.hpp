#pragma once

class BSLSMLandscapeExtended : public RE::BSLightingShaderMaterialLandscape
{
public:
	void CopyMembers(RE::BSShaderMaterial* that) override;
	void ClearTextures() override;
	uint32_t GetTextures(RE::NiSourceTexture** textures) override;

	// Texture storage
	std::array<RE::NiPointer<RE::NiSourceTexture>, 6> landscapeDiffuseTex;
	std::array<RE::NiPointer<RE::NiSourceTexture>, 6> landscapeNormalTex;
	std::array<RE::NiPointer<RE::NiSourceTexture>, 6> landscapeGlowTex;
	std::array<RE::NiPointer<RE::NiSourceTexture>, 6> landscapeHeightTex;
	std::array<RE::NiPointer<RE::NiSourceTexture>, 6> landscapeEnvTex;
	std::array<RE::NiPointer<RE::NiSourceTexture>, 6> landscapeEnvMaskTex;
	std::array<RE::NiPointer<RE::NiSourceTexture>, 6> landscapeSubsurfaceTex;
	std::array<RE::NiPointer<RE::NiSourceTexture>, 6> landscapeBacklightTex;
};
