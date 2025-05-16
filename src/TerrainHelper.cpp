#include "TerrainHelper.h"

using namespace std;

// statics
std::shared_mutex TerrainHelper::extendedSlotsMutex;
std::unordered_map<uint32_t, TerrainHelper::ExtendedSlots> TerrainHelper::extendedSlots;
std::unordered_set<std::string> TerrainHelper::texturesErrorLogged;
RE::BGSTextureSet* TerrainHelper::defaultLandTexture = nullptr;

void TerrainHelper::BSLightingShader_SetupMaterial(RE::BSLightingShader* shader, RE::BSLightingShaderMaterialBase const* material) {
    if (material == nullptr) {
        // material is null
        return;
    }

    ExtendedSlots materialBase;
    {
        const shared_lock lock(extendedSlotsMutex);
        if (!extendedSlots.contains(material->hashKey)) {
            // hash does not exists
            return;
        }

        materialBase = extendedSlots[material->hashKey];
    }

    // get renderer and shadow state
    auto shadowState = RE::BSGraphics::RendererShadowState::GetSingleton();

    const auto& stateData = RE::BSGraphics::State::GetSingleton()->GetRuntimeData();

    static constexpr size_t ParallaxStartIndex = 16;  // 16-21

    // Populate extended slots
    for (uint32_t textureI = 0; textureI < 6; ++textureI) {
        const uint32_t heightIndex = ParallaxStartIndex + textureI;
        if (materialBase.parallax[textureI] != nullptr && materialBase.parallax[textureI] != stateData.defaultTextureNormalMap) {
            shadowState->SetPSTexture(heightIndex, materialBase.parallax[textureI]->rendererTexture);
        }
        else {
            // set default texture, which is empty
            shadowState->SetPSTexture(heightIndex, nullptr);
        }
    }
}

void TerrainHelper::TESObjectLAND_SetupMaterial(RE::TESObjectLAND* land) {
    for (uint32_t quadI = 0; quadI < 4; ++quadI) {
        // Get hash key of vanilla material
        uint32_t hashKey = 0;

        if (land->loadedData->mesh[quadI] == nullptr) {
            // continue if cannot find mesh
            continue;
        }

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

        // Create array of texture sets (6 tiles)
        std::array<RE::BGSTextureSet*, 6> textureSets;
        auto defTexture = land->loadedData->defQuadTextures[quadI];
        if (defTexture != nullptr && defTexture->formID != 0) {
            textureSets[0] = defTexture->textureSet;
        }
        else {
            // this is a default texture
            textureSets[0] = defaultLandTexture;
        }
        for (uint32_t textureI = 0; textureI < 5; ++textureI) {
            auto curTexture = land->loadedData->quadTextures[quadI][textureI];
            if (curTexture == nullptr) {
                textureSets[textureI + 1] = nullptr;
                continue;
            }

            if (curTexture->formID == 0) {
                // this is a default texture
                textureSets[textureI + 1] = defaultLandTexture;
            }
            else {
                textureSets[textureI + 1] = land->loadedData->quadTextures[quadI][textureI]->textureSet;
            }
        }

        // Assign textures to material
        {
            const unique_lock lock(extendedSlotsMutex);
            if (!extendedSlots.contains(hashKey)) {
                extendedSlots[hashKey] = {};
            }

            for (uint32_t textureI = 0; textureI < 6; ++textureI) {
                if (textureSets[textureI] == nullptr) {
                    continue;
                }

                auto txSet = textureSets[textureI];
                if (txSet->GetTexturePath(static_cast<RE::BSTextureSet::Texture>(3)) != nullptr) {
                    txSet->SetTexture(static_cast<RE::BSTextureSet::Texture>(3), extendedSlots[hashKey].parallax[textureI]);

                    if (extendedSlots[hashKey].parallax[textureI] == RE::BSGraphics::State::GetSingleton()->GetRuntimeData().defaultTextureNormalMap) {
                        // this file was not found
                        if (get<1>(texturesErrorLogged.insert(txSet->GetTexturePath(static_cast<RE::BSTextureSet::Texture>(3))))) {
                            spdlog::error("Unable to find parallax map {} while setting up material", txSet->GetTexturePath(static_cast<RE::BSTextureSet::Texture>(3)));
                        }
                    }
                }
            }
        }
    }
}

void TerrainHelper::onDataLoaded() {
    // Get the default landscape texture set for terrain helper
    const auto thDefaultLandTexSet = RE::TESForm::LookupByEditorID<RE::BGSTextureSet>("LandscapeDefault");

    // Get game default texture set
    const auto skyrimDefaultLand = GetDefaultLandTexture();

    RE::BGSTextureSet* skyrimDefaultLandTexSet = nullptr;
    if (thDefaultLandTexSet != nullptr) {
        spdlog::info("LandscapeDefault EDID texture set found");
        defaultLandTexture = thDefaultLandTexSet;

        if (skyrimDefaultLand != nullptr) {
			spdlog::info("Replacing skyrim default texture set with LandscapeDefault");
            skyrimDefaultLand->textureSet = thDefaultLandTexSet;
        }
        else {
			spdlog::warn("Skyrim default texture set not found, diffuse and normal slots from LandscapeDefault record will not function");
        }
    }
    else {
        spdlog::warn("LandscapeDefault EDID texture set not found, extended slots will not work for default tiles");
    }
}

RE::TESLandTexture* TerrainHelper::GetDefaultLandTexture() {
    static const auto defaultLandTextureAddress = REL::Relocation<RE::TESLandTexture**>(RELOCATION_ID(514783, 400936));
    return *defaultLandTextureAddress;
}