#pragma once

#include "OpenGLWindow.hpp"
#include "GraphicsAPI.hpp"

#include "unordered_map"
#include "glm/mat4x4.hpp" // mat4, dmat4

struct GladGLContext;

// Implements GraphicsAPI. Takes DrawCalls and clears them in draw() using an OpenGL rendering pipeline.
class OpenGLAPI : public GraphicsAPI
{
public:
	OpenGLAPI();
	~OpenGLAPI();

	// Initialising OpenGLAPI requires an OpenGLWindow to be created beforehand as GLAD requires a context to be set for its initialisation.
	void draw() 								override;
	void onFrameStart() 						override;
	void setView(const glm::mat4& pViewMatrix)	override;
private:
	void initialiseMesh(const Mesh &pMesh) 		override;


	// Defines HOW a Mesh should be rendered, has a 1:1 relationship with mesh
	struct DrawInfo
	{
		static const unsigned int invalidHandle = 0;
		enum class DrawMethod
		{
			Indices,
			Array,
			Null
		};

		// OpenGL handles
		unsigned int mShaderID 		= invalidHandle;
		unsigned int mVAO 			= invalidHandle;
		unsigned int mVBO 			= invalidHandle;
		unsigned int mEBO 			= invalidHandle;
		unsigned int mDrawMode 		= invalidHandle;
		size_t mDrawSize 			= invalidHandle; // Cached size of data used in draw, either size of Mesh positions or indices
		DrawMethod mDrawMethod 		= DrawMethod::Null;
	};
	const DrawInfo& getDrawInfo(const MeshID& pMeshID);
	std::unordered_map<MeshID, DrawInfo> mMeshManager; 		// Mapping of a Mesh to how it should be drawn.

	void setClearColour(const float& pRed, const float& pGreen, const float& pBlue);
	void clearBuffers();

	void initialiseTextures();
	void initialiseShaders();
	unsigned int loadTexture(const std::string &pFileName);
	unsigned int loadShader(const std::string &pVertexShader, const std::string &pFragmentShader);
	enum class ProgramType
	{
		VertexShader,
		FragmentShader,
		ShaderProgram
	};
	bool hasCompileErrors(const unsigned int pProgramID, const ProgramType &pType);
	int getPolygonMode(const DrawCall::DrawMode& pDrawMode);

	const int cOpenGLVersionMajor, cOpenGLVersionMinor;
	const size_t cMaxTextureUnits; // The limit on the number of texture units available in the shaders using sampler2D
	unsigned int mRegularShader;
	unsigned int mTextureShader;
	float mWindowClearColour[3]; // Colour the window will be cleared with in RGB 0-1.
	glm::mat4 mViewMatrix; // Cached view matrix the active camera has been set to. Updated via callback using setView()

	OpenGLWindow mWindow;
	GladGLContext* mGLADContext; // Depends on OpenGLWindow being initialised first. Must be declared after mWindow.
	static GladGLContext* initialiseGLAD(); // Requires a GLFW window to be set as current context, done in OpenGLWindow constructor
	static void windowSizeCallback(GLFWwindow *pWindow, int pWidth, int pHeight); // Callback required by GLFW to be static/global.
};