#include "BSLSMLandscapeExtended.hpp"

void BSLSMLandscapeExtended::CopyMembers(RE::BSShaderMaterial* that)
{
	RE::BSLightingShaderMaterialBase::CopyMembers(that);

	auto* newMat = static_cast<BSLSMLandscapeExtended*>(that);

	for (uint32_t textureIndex = 0; textureIndex < 6; ++textureIndex) {
		newMat->landscapeDiffuseTex[textureIndex] = landscapeDiffuseTex[textureIndex];
		newMat->landscapeNormalTex[textureIndex] = landscapeNormalTex[textureIndex];
		newMat->landscapeGlowTex[textureIndex] = landscapeGlowTex[textureIndex];
		newMat->landscapeHeightTex[textureIndex] = landscapeHeightTex[textureIndex];
		newMat->landscapeEnvTex[textureIndex] = landscapeEnvTex[textureIndex];
		newMat->landscapeEnvMaskTex[textureIndex] = landscapeEnvMaskTex[textureIndex];
		newMat->landscapeSubsurfaceTex[textureIndex] = landscapeSubsurfaceTex[textureIndex];
		newMat->landscapeBacklightTex[textureIndex] = landscapeBacklightTex[textureIndex];
	}
}

void BSLSMLandscapeExtended::ClearTextures()
{
	BSLightingShaderMaterialLandscape::ClearTextures();
	for (auto& texture : landscapeDiffuseTex) {
    texture.reset();
  }
	for (auto& texture : landscapeNormalTex) {
    texture.reset();
  }
	for (auto& texture : landscapeGlowTex) {
    texture.reset();
  }
	for (auto& texture : landscapeHeightTex) {
    texture.reset();
  }
	for (auto& texture : landscapeEnvTex) {
    texture.reset();
  }
	for (auto& texture : landscapeEnvMaskTex) {
    texture.reset();
  }
	for (auto& texture : landscapeSubsurfaceTex) {
    texture.reset();
  }
	for (auto& texture : landscapeBacklightTex) {
    texture.reset();
  }
}

uint32_t BSLSMLandscapeExtended::GetTextures(RE::NiSourceTexture** textures)
{
  uint32_t numTextures = 0;
  for (uint32_t textureIndex = 0; textureIndex < 6; ++textureIndex) {
    if (landscapeDiffuseTex[textureIndex]) {
      textures[numTextures++] = landscapeDiffuseTex[textureIndex].get();
    }
    if (landscapeNormalTex[textureIndex]) {
      textures[numTextures++] = landscapeNormalTex[textureIndex].get();
    }
    if (landscapeGlowTex[textureIndex]) {
      textures[numTextures++] = landscapeGlowTex[textureIndex].get();
    }
    if (landscapeHeightTex[textureIndex]) {
      textures[numTextures++] = landscapeHeightTex[textureIndex].get();
    }
    if (landscapeEnvTex[textureIndex]) {
      textures[numTextures++] = landscapeEnvTex[textureIndex].get();
    }
    if (landscapeEnvMaskTex[textureIndex]) {
      textures[numTextures++] = landscapeEnvMaskTex[textureIndex].get();
    }
    if (landscapeSubsurfaceTex[textureIndex]) {
      textures[numTextures++] = landscapeSubsurfaceTex[textureIndex].get();
    }
    if (landscapeBacklightTex[textureIndex]) {
      textures[numTextures++] = landscapeBacklightTex[textureIndex].get();
    }
  }
  return numTextures;
}