#pragma once

#include "Mesh.hpp"

#include "glm/vec3.hpp"	// vec3, bvec3, dvec3, ivec3 and uvec3

#include "unordered_map"
#include "functional"

namespace std
{
    namespace filesystem
    {
        class path;
    }
}

struct aiNode;
struct aiScene;
struct aiMesh;
struct aiMaterial;
class TextureManager;

class MeshManager
{
public:
    MeshManager(TextureManager& pTextureManager)
    : mTextureManager(pTextureManager)
    {
        buildMeshes();
    }

    inline void ForEach(const std::function<void(const Mesh&)>& pFunction) const
    {
        for (const auto& mesh : mMeshes)
            pFunction(mesh.second);
    }

	MeshID getMeshID(const std::string& pMeshName);

    // Loads model data using ASSIMP from pFilePath.
    MeshID loadModel(const std::filesystem::path& pFilePath);

private:
    std::unordered_map<MeshID, Mesh> mMeshes;
    std::unordered_map<std::string, MeshID> mMeshNames;
    TextureManager& mTextureManager; // Owned by Renderer.
    size_t activeMeshes = 0;

    // Set the mID of the mesh and its children recursively.
    void setID(Mesh& pMesh, const bool& pRootMesh);

    void addMesh(Mesh& pMesh);
    void buildMeshes();
    bool isMeshValid(const Mesh& pMesh);

    // Recursively travel all the aiNodes and extract the per-vertex data into a Zephyr mesh object.
    void processNode(Mesh& pParentMesh, aiNode* pNode, const aiScene* pScene);
    // Load assimp mesh data into a Zephyr Mesh.
    void processData(Mesh& pMesh, const aiMesh *pAssimpMesh, const aiScene *pAssimpScene);
    // Returns all the textures for this material and purpose.
    void processTextures(Mesh &pMesh, aiMaterial* pMaterial, const Texture::Purpose& pPurpose);
};