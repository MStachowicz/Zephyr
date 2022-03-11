#pragma once

#include "Mesh.hpp"
#include "Texture.hpp"
#include "LightManager.hpp"
#include "Camera.hpp"
#include "DrawCall.hpp"
#include "ComponentManager.hpp"

class GraphicsAPI;

// Submits DrawCalls to it's GraphicsAPI which itself implements the rendering pipeline being used.
class Renderer
{
public:
	Renderer();
	~Renderer();

	void onFrameStart();
	void draw();
	void postDraw();

	int drawCount = 0;

	Camera& getCamera() { return mCamera; }

private:
	// Order of initialisation is important here:

	// Stores all the meshes derived graphics APIs can draw.
	MeshManager mMeshManager;
	TextureManager mTextureManager;
	LightManager mLightManager;
	GraphicsAPI *mOpenGLAPI;
	Camera mCamera;

	DrawCall lightPosition;
	ECS::ComponentManager<DrawCall> mDrawCalls;
};