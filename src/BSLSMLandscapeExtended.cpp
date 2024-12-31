#include "BSLSMLandscapeExtended.hpp"

BSLSMLandscapeExtended::BSLSMLandscapeExtended()
{
}

BSLSMLandscapeExtended::~BSLSMLandscapeExtended()
{
}

RE::BSShaderMaterial* BSLSMLandscapeExtended::Create()
{
  return new BSLSMLandscapeExtended();
}

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

RE::BSLightingShaderMaterialBase::Feature BSLSMLandscapeExtended::GetFeature() const
{
  return RE::BSLightingShaderMaterialBase::Feature::kMultiTexLandLODBlend;
}

void BSLSMLandscapeExtended::ReceiveValuesFromRootMaterial(bool a_skinned, bool a_rimLighting, bool a_softLighting, bool a_backLighting, bool a_MSN)
{
  BSLightingShaderMaterialBase::ReceiveValuesFromRootMaterial(a_skinned, a_rimLighting, a_softLighting, a_backLighting, a_MSN);
  const auto& stateData = RE::BSGraphics::State::GetSingleton()->GetRuntimeData();
  if (terrainOverlayTexture == nullptr) {
    terrainOverlayTexture = stateData.defaultTextureNormalMap;
  }
  if (terrainNoiseTexture == nullptr) {
    terrainNoiseTexture = stateData.defaultTextureNormalMap;
  }
  for (uint32_t textureIndex = 0; textureIndex < numLandscapeTextures; ++textureIndex) {
    if (landscapeDiffuseTex[textureIndex] == nullptr) {
      landscapeDiffuseTex[textureIndex] = stateData.defaultTextureBlack;
    }
    if (landscapeNormalTex[textureIndex] == nullptr) {
      landscapeNormalTex[textureIndex] = stateData.defaultTextureNormalMap;
    }
    if (landscapeGlowTex[textureIndex] == nullptr) {
      landscapeGlowTex[textureIndex] = stateData.defaultTextureBlack;
    }
    if (landscapeHeightTex[textureIndex] == nullptr) {
      landscapeHeightTex[textureIndex] = stateData.defaultTextureBlack;
    }
    if (landscapeEnvTex[textureIndex] == nullptr) {
      landscapeEnvTex[textureIndex] = stateData.defaultTextureBlack;
    }
    if (landscapeEnvMaskTex[textureIndex] == nullptr) {
      landscapeEnvMaskTex[textureIndex] = stateData.defaultTextureBlack;
    }
    if (landscapeSubsurfaceTex[textureIndex] == nullptr) {
      landscapeSubsurfaceTex[textureIndex] = stateData.defaultTextureBlack;
    }
    if (landscapeBacklightTex[textureIndex] == nullptr) {
      landscapeBacklightTex[textureIndex] = stateData.defaultTextureBlack;
    }
  }
}

void BSLSMLandscapeExtended::ClearTextures()
{
	RE::BSLightingShaderMaterialBase::ClearTextures();
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
  for (uint32_t textureIndex = 0; textureIndex < numLandscapeTextures; ++textureIndex) {
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
  if (terrainOverlayTexture) {
    textures[numTextures++] = terrainOverlayTexture.get();
  }
  if (terrainNoiseTexture) {
    textures[numTextures++] = terrainNoiseTexture.get();
  }
  return numTextures;
}
