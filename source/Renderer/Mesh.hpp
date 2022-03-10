#pragma once

#include "Logger.hpp"
#include "vector"
#include "string"

#include "glm/vec3.hpp"	// vec3, bvec3, dvec3, ivec3 and uvec3

typedef unsigned int MeshID; // The unique ID used as an identifier of a Mesh.

// Mesh stores all per-vertex data to represent a 3D object.
struct Mesh
{
    Mesh() : mID(++nextMesh) {}

    const MeshID mID; // Unique ID to map this mesh to DrawInfo within the graphics context being used.
    std::string mName;
    std::vector<std::string> mAttributes;

    std::vector<float> mVertices;           // Per-vertex position attributes.
    std::vector<float> mNormals;            // Per-vertex normal attributes.
    std::vector<float> mColours;            // Per-vertex colour attributes.
    std::vector<float> mTextureCoordinates; // Per-vertex texture mapping.
    std::vector<int> mIndices;              // Allows indexing into the mVertices and mColours data to specify an indexed draw order.
private:
    static inline MeshID nextMesh = 0;
};

class MeshManager
{
private:
    std::unordered_map<MeshID, Mesh> mMeshes;
    std::unordered_map<std::string, MeshID> mMeshNames;

public:
    MeshManager()
    {
        buildMeshes();
    }

    inline void ForEach(const std::function<void(const Mesh&)>& pFunction) const
    {
        for (const auto& mesh : mMeshes)
            pFunction(mesh.second);
    }

	MeshID getMeshID(const std::string& pMeshName)
	{
		const auto it = mMeshNames.find(pMeshName);
		ZEPHYR_ASSERT(it != mMeshNames.end(), "Searching for a mesh that does not exist in Mesh store.");
		return it->second;
	}

private:
    void addMesh(const Mesh& pMesh)
    {
        mMeshes.emplace(std::make_pair(pMesh.mID, pMesh));
        mMeshNames.emplace(std::make_pair(pMesh.mName, pMesh.mID));
    }

    void buildMeshes() // Populates mMeshes with some commonly used shapes
    {
        { // 2D TRIANGLE
            Mesh mesh;
            mesh.mName = "2DTriangle";
            mesh.mAttributes = {"Position", "Colour", "Texture Coordinates"};
            mesh.mVertices = {
                -1.0f, -1.0f, 0.0f, // Left
                1.0f, -1.0f, 0.0f,  // Right
                0.0f, 1.0f, 0.0f,   // Top
            };
            mesh.mColours = {
                0.0f, 1.0f, 0.0f,
                0.0f, 0.0f, 1.0f,
                1.0f, 0.0f, 0.0f};
            mesh.mTextureCoordinates = {
                0.0f, 0.0f,
                1.0f, 0.0f,
                0.5f, 1.0f};
            addMesh(mesh);
        }
        { // 2D SQUARE
            Mesh mesh;
            mesh.mName = "2DSquare";
            mesh.mAttributes = {"Position", "Colour", "Texture Coordinates"};
            mesh.mVertices = {
                -1.0f, 1.0f, 0.0f,  // Top left
                -1.0f, -1.0f, 0.0f, // Bottom left
                1.0f, -1.0f, 0.0f,  // Bottom right
                1.0f, 1.0f, 0.0f,   // Top right
            };
            mesh.mColours = {
                0.0f, 0.0f, 1.0f,
                0.0f, 1.0f, 0.0f,
                1.0f, 0.0f, 0.0f,
                1.0f, 1.0f, 0.0f};
            mesh.mTextureCoordinates = {
                1.0f, 1.0f,
                1.0f, 0.0f,
                0.0f, 0.0f,
                0.0f, 1.0f};
            mesh.mIndices = {
                0, 1, 3, // first triangle
                1, 2, 3  // second triangle
            };
            addMesh(mesh);
        }
        { // 3D CUBE (supported attributes: VertexPosition, VertexColour) + Indexed drawing (EBO)
            //	   0----------1
            //	  /|         /|
            //	 / |        / |
            //	2----------3  |
            //	|  |       |  |
            //	|  4-------|--5
            //	| /        | /
            //	|/         |/
            //	6----------7
            Mesh mesh;
            mesh.mName = "3DCubeIndex";
            mesh.mAttributes = {"Position", "Colour"};
            mesh.mIndices = {
                0, 1, 2, // Top 1
                1, 2, 3, // Top 2
                2, 3, 6, // Front 1
                3, 6, 7, // Front 2
                3, 1, 7, // Right 1
                7, 5, 1, // Right 2
                0, 1, 4, // Back 1
                4, 5, 1, // Back 2
                2, 0, 6, // Left 1
                6, 4, 0, // Left 2
                4, 6, 7, // Bottom 1
                7, 5, 4  // Bottom 2
            };
            mesh.mVertices = {
                -1.0f, 1.0f, -1.0f,  // 0
                1.0f, 1.0f, -1.0f,   // 1
                -1.0f, 1.0f, 1.0f,   // 2
                1.0f, 1.0f, 1.0f,    // 3
                -1.0f, -1.0f, -1.0f, // 4
                1.0f, -1.0f, -1.0f,  // 5
                -1.0f, -1.0f, 1.0f,  // 6
                1.0f, -1.0f, 1.0f    // 7
            };
            mesh.mColours = {
                0.0f, 0.0f, 1.0f, // 0
                0.0f, 1.0f, 0.0f, // 1
                1.0f, 0.0f, 0.0f, // 2
                1.0f, 1.0f, 0.0f, // 3
                1.0f, 1.0f, 0.0f, // 4
                1.0f, 1.0f, 0.0f, // 5
                1.0f, 1.0f, 0.0f, // 6
                1.0f, 1.0f, 0.0f  // 7
            };
            // mesh.mTextureCoordinates =
            //{ - Requires support for 3D textures - one for each face
            //	x,x,x,// 0
            //	x,x,x,// 1
            //	x,x,x,// 2
            //	x,x,x,// 3
            //	x,x,x,// 4
            //	x,x,x,// 5
            //	x,x,x,// 6
            //	x,x,x// 7
            //	};
            addMesh(mesh);
        }
        { // 3D CUBE (supported vertex attributes: Position, Texture coordinates(2D), Normal, Colour)
            Mesh mesh;
            mesh.mName = "3DCube";
            mesh.mAttributes = {"Position", "Texture Coordinate", "Normal", "Colour"};
            mesh.mVertices = {
                -0.5f, -0.5f, -0.5f,
                0.5f, -0.5f, -0.5f,
                0.5f, 0.5f, -0.5f,
                0.5f, 0.5f, -0.5f,
                -0.5f, 0.5f, -0.5f,
                -0.5f, -0.5f, -0.5f,

                -0.5f, -0.5f, 0.5f,
                0.5f, -0.5f, 0.5f,
                0.5f, 0.5f, 0.5f,
                0.5f, 0.5f, 0.5f,
                -0.5f, 0.5f, 0.5f,
                -0.5f, -0.5f, 0.5f,

                -0.5f, 0.5f, 0.5f,
                -0.5f, 0.5f, -0.5f,
                -0.5f, -0.5f, -0.5f,
                -0.5f, -0.5f, -0.5f,
                -0.5f, -0.5f, 0.5f,
                -0.5f, 0.5f, 0.5f,

                0.5f, 0.5f, 0.5f,
                0.5f, 0.5f, -0.5f,
                0.5f, -0.5f, -0.5f,
                0.5f, -0.5f, -0.5f,
                0.5f, -0.5f, 0.5f,
                0.5f, 0.5f, 0.5f,

                -0.5f, -0.5f, -0.5f,
                0.5f, -0.5f, -0.5f,
                0.5f, -0.5f, 0.5f,
                0.5f, -0.5f, 0.5f,
                -0.5f, -0.5f, 0.5f,
                -0.5f, -0.5f, -0.5f,

                -0.5f, 0.5f, -0.5f,
                0.5f, 0.5f, -0.5f,
                0.5f, 0.5f, 0.5f,
                0.5f, 0.5f, 0.5f,
                -0.5f, 0.5f, 0.5f,
                -0.5f, 0.5f, -0.5f};
            mesh.mNormals = {
                0.0f, 0.0f, -1.0f,
                0.0f, 0.0f, -1.0f,
                0.0f, 0.0f, -1.0f,
                0.0f, 0.0f, -1.0f,
                0.0f, 0.0f, -1.0f,
                0.0f, 0.0f, -1.0f,
                0.0f, 0.0f, 1.0f,
                0.0f, 0.0f, 1.0f,
                0.0f, 0.0f, 1.0f,
                0.0f, 0.0f, 1.0f,
                0.0f, 0.0f, 1.0f,
                0.0f, 0.0f, 1.0f,
                -1.0f, 0.0f, 0.0f,
                -1.0f, 0.0f, 0.0f,
                -1.0f, 0.0f, 0.0f,
                -1.0f, 0.0f, 0.0f,
                -1.0f, 0.0f, 0.0f,
                -1.0f, 0.0f, 0.0f,
                1.0f, 0.0f, 0.0f,
                1.0f, 0.0f, 0.0f,
                1.0f, 0.0f, 0.0f,
                1.0f, 0.0f, 0.0f,
                1.0f, 0.0f, 0.0f,
                1.0f, 0.0f, 0.0f,
                0.0f, -1.0f, 0.0f,
                0.0f, -1.0f, 0.0f,
                0.0f, -1.0f, 0.0f,
                0.0f, -1.0f, 0.0f,
                0.0f, -1.0f, 0.0f,
                0.0f, -1.0f, 0.0f,
                0.0f, 1.0f, 0.0f,
                0.0f, 1.0f, 0.0f,
                0.0f, 1.0f, 0.0f,
                0.0f, 1.0f, 0.0f,
                0.0f, 1.0f, 0.0f,
                0.0f, 1.0f, 0.0f};
            mesh.mTextureCoordinates = {
                0.0f, 0.0f,
                1.0f, 0.0f,
                1.0f, 1.0f,
                1.0f, 1.0f,
                0.0f, 1.0f,
                0.0f, 0.0f,
                0.0f, 0.0f,
                1.0f, 0.0f,
                1.0f, 1.0f,
                1.0f, 1.0f,
                0.0f, 1.0f,
                0.0f, 0.0f,
                1.0f, 0.0f,
                1.0f, 1.0f,
                0.0f, 1.0f,
                0.0f, 1.0f,
                0.0f, 0.0f,
                1.0f, 0.0f,
                1.0f, 0.0f,
                1.0f, 1.0f,
                0.0f, 1.0f,
                0.0f, 1.0f,
                0.0f, 0.0f,
                1.0f, 0.0f,
                0.0f, 1.0f,
                1.0f, 1.0f,
                1.0f, 0.0f,
                1.0f, 0.0f,
                0.0f, 0.0f,
                0.0f, 1.0f,
                0.0f, 1.0f,
                1.0f, 1.0f,
                1.0f, 0.0f,
                1.0f, 0.0f,
                0.0f, 0.0f,
                0.0f, 1.0f};
            const glm::vec3 colour = glm::vec3(0.0f, 0.0f, 1.0f);
            mesh.mColours = {
                colour.r, colour.g, colour.b,
                colour.r, colour.g, colour.b,
                colour.r, colour.g, colour.b,
                colour.r, colour.g, colour.b,
                colour.r, colour.g, colour.b,
                colour.r, colour.g, colour.b,
                colour.r, colour.g, colour.b,
                colour.r, colour.g, colour.b,
                colour.r, colour.g, colour.b,
                colour.r, colour.g, colour.b,
                colour.r, colour.g, colour.b,
                colour.r, colour.g, colour.b,
                colour.r, colour.g, colour.b,
                colour.r, colour.g, colour.b,
                colour.r, colour.g, colour.b,
                colour.r, colour.g, colour.b,
                colour.r, colour.g, colour.b,
                colour.r, colour.g, colour.b,
                colour.r, colour.g, colour.b,
                colour.r, colour.g, colour.b,
                colour.r, colour.g, colour.b,
                colour.r, colour.g, colour.b,
                colour.r, colour.g, colour.b,
                colour.r, colour.g, colour.b,
                colour.r, colour.g, colour.b,
                colour.r, colour.g, colour.b,
                colour.r, colour.g, colour.b,
                colour.r, colour.g, colour.b,
                colour.r, colour.g, colour.b,
                colour.r, colour.g, colour.b,
                colour.r, colour.g, colour.b,
                colour.r, colour.g, colour.b,
                colour.r, colour.g, colour.b,
                colour.r, colour.g, colour.b,
                colour.r, colour.g, colour.b,
                colour.r, colour.g, colour.b
            };
            addMesh(mesh);
        }

        for (const auto &mesh : mMeshes)
        {
            ZEPHYR_ASSERT(!mesh.second.mVertices.empty(), "A mesh must have position data.");
            ZEPHYR_ASSERT(!mesh.second.mName.empty(), "A mesh must have a name.");

            // if (!mesh.second.mNormals.empty()) // Colour data must have size parity with position
            //	ZEPHYR_ASSERT(mesh.second.mNormals.size() == mesh.second.mVertices.size(), ("Size of colour data ({}) does not match size of position data ({}), cannot buffer the colour data", mesh.second.mColours.size(), mesh.second.mVertices.size()));
            if (!mesh.second.mColours.empty()) // Colour data must have size parity with position
                ZEPHYR_ASSERT(mesh.second.mColours.size() == mesh.second.mVertices.size(), ("Size of colour data ({}) does not match size of position data ({}), cannot buffer the colour data", mesh.second.mColours.size(), mesh.second.mVertices.size()));
        }
    }
};