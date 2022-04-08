#define IMGUI_USER_CONFIG "ImGuiConfig.hpp"
#include "imgui.h"

#include "Renderer.hpp"
#include "OpenGLAPI.hpp"
#include "Logger.hpp"
#include "Timer.hpp"

#include "cmath"
#include "numeric"
#include "chrono"

Renderer::Renderer()
: mDrawCount(0)
, mTargetFPS(60)
, mTextureManager()
, mMeshManager(mTextureManager)
, mLightManager()
, mOpenGLAPI(new OpenGLAPI(mLightManager))
, mCamera(glm::vec3(0.0f, 0.0f, 7.0f), std::bind(&GraphicsAPI::setView, mOpenGLAPI, std::placeholders::_1), std::bind(&GraphicsAPI::setViewPosition, mOpenGLAPI, std::placeholders::_1))
, mRenderImGui(true)
, mShowFPSPlot(false)
, mUseRawPerformanceData(false)
, mDataSmoothingFactor(0.1f)
, mFPSSampleSize(120)
, mAverageFPS(0)
, mCurrentFPS(0)
, mTimeSinceLastDraw(0)
, mImGuiRenderTimeTakenMS(0)
, mDrawTimeTakenMS(0)
{
	mMeshManager.ForEach([this](const auto &mesh) { mOpenGLAPI->initialiseMesh(mesh); }); // Depends on mShaders being initialised.
	mTextureManager.ForEach([this](const auto &texture) { mOpenGLAPI->initialiseTexture(texture); });

	lightPosition.mScale 		= glm::vec3(0.1f);
	lightPosition.mMesh 		= mMeshManager.getMeshID("3DCube");
	lightPosition.mColour		= glm::vec3(1.f);
	lightPosition.mDrawStyle 	= DrawStyle::UniformColour;

	for (size_t i = 0; i < 25; i++)
	{
		DrawCall &drawCall = mDrawCalls.Create(ECS::CreateEntity());
		drawCall.mPosition = glm::vec3(-1.0f, 0.0f, 10.0f - i);
		drawCall.mScale = glm::vec3(0.5f);
		drawCall.mMesh = mMeshManager.getMeshID("backpack");
		drawCall.mDrawStyle = DrawStyle::LightMap;
		drawCall.mDiffuseTextureID = mTextureManager.getTextureID("diffuse");
		drawCall.mSpecularTextureID = mTextureManager.getTextureID("specular");
		drawCall.mShininess = 64.f;
	}
	{
		DrawCall &drawCall = mDrawCalls.Create(ECS::CreateEntity());
		drawCall.mPosition = glm::vec3(8.0f, 0.0f, 0.0f);
		drawCall.mRotation = glm::vec3(-10.0f, 230.0f, -15.0f);
		drawCall.mScale = glm::vec3(0.4f);
		drawCall.mMesh = mMeshManager.getMeshID("xian");
		drawCall.mDrawStyle = DrawStyle::LightMap;
		drawCall.mDiffuseTextureID = mTextureManager.getTextureID("Base_Color");
		drawCall.mSpecularTextureID = mTextureManager.getTextureID("black");
		drawCall.mShininess = 64.f;
	}

	std::array<glm::vec3, 10> cubePositions = {
    glm::vec3( 0.0f,  0.0f,  -30.0f),
    glm::vec3( 2.0f,  5.0f, -15.0f),
    glm::vec3(-1.5f, -2.2f, -2.5f),
    glm::vec3(-3.8f, -2.0f, -12.3f),
    glm::vec3( 2.4f, -0.4f, -3.5f),
    glm::vec3(-1.7f,  3.0f, -7.5f),
    glm::vec3( 1.3f, -2.0f, -2.5f),
    glm::vec3( 1.5f,  2.0f, -2.5f),
    glm::vec3( 1.5f,  0.2f, -1.5f),
    glm::vec3(-1.3f,  1.0f, -1.5f)};

	for (size_t i = 0; i < cubePositions.size(); i++)
	{
		DrawCall &drawCall 	= mDrawCalls.Create(ECS::CreateEntity());
		drawCall.mPosition 	= cubePositions[i];
		drawCall.mMesh 		= mMeshManager.getMeshID("3DCube");

		drawCall.mDrawStyle 	   	= DrawStyle::LightMap;
		drawCall.mDiffuseTextureID 	= mTextureManager.getTextureID("metalContainerDiffuse");
		drawCall.mSpecularTextureID = mTextureManager.getTextureID("metalContainerSpecular");
		drawCall.mShininess 		= 64.f;
	}
}

Renderer::~Renderer()
{
	delete mOpenGLAPI;
}

void Renderer::onFrameStart(const std::chrono::microseconds& pTimeSinceLastDraw)
{
	mTimeSinceLastDraw 	= static_cast<float>(pTimeSinceLastDraw.count()) / 1000.0f;

	if (mUseRawPerformanceData)
		mCurrentFPS = 1.0f / (static_cast<float>((pTimeSinceLastDraw.count()) / 1000000.0f));
	else
		mCurrentFPS = (mDataSmoothingFactor * (1.0f / (static_cast<float>((pTimeSinceLastDraw.count()) / 1000000.0f)))) + (1.0f - mDataSmoothingFactor) * mCurrentFPS;

	mOpenGLAPI->onFrameStart();

	{ // Setup lights in GraphicsAPI
		mLightManager.getPointLights().ForEach([this](const PointLight& pPointLight) { mOpenGLAPI->draw(pPointLight); });
		mLightManager.getDirectionalLights().ForEach([this](const DirectionalLight& pDirectionalLight) { mOpenGLAPI->draw(pDirectionalLight); });
		mLightManager.getSpotlightsLights().ForEach([this](const SpotLight& pSpotLight) { mOpenGLAPI->draw(pSpotLight); });
	}
}

void Renderer::draw(const std::chrono::microseconds& pTimeSinceLastDraw)
{
	Stopwatch stopwatch;

	onFrameStart(pTimeSinceLastDraw);
	{ // Draw all meshes via DrawCalls
		mDrawCalls.ForEach([&](const DrawCall &pDrawCall)
		{
			mOpenGLAPI->draw(pDrawCall);
		});

		if (mLightManager.mRenderLightPositions)
		{
			mLightManager.getPointLights().ForEach([&](const PointLight &pPointLight)
			{
				lightPosition.mPosition = pPointLight.mPosition;
				lightPosition.mColour	= pPointLight.mColour;
				mOpenGLAPI->draw(lightPosition);
			});
		}
	}
	postDraw();

	mDrawCount++;
	mDrawTimeTakenMS = stopwatch.getTime<std::milli, float>();
}

void Renderer::postDraw()
{
	renderImGui();
	mOpenGLAPI->postDraw(); // Swaps the buffers, must be called after draw
}

void Renderer::renderImGui()
{
	// Render all ImGui from here.
	// Regardless of mRenderImGui, we call newImGuiFrame() and renderImGuiFrame() to allow showing performance window.
	Stopwatch stopWatch;

	mOpenGLAPI->newImGuiFrame();

	if (mRenderImGui)
	{
		if (ImGui::Begin("Render options", nullptr))
		{
			ImGui::Checkbox("Render light positions", &mLightManager.mRenderLightPositions);
		}
		ImGui::End();

		static ImGuiWindowFlags window_flags = ImGuiWindowFlags_AlwaysAutoResize;
		if (ImGui::Begin("ImGui options", nullptr, window_flags))
		{
			ImGuiIO &io = ImGui::GetIO();
			ImGui::SliderFloat("FontGlobalScale", &io.FontGlobalScale, 0.1f, 5.f);
			ImGui::SliderFloat2("DisplaySize", &io.DisplaySize.x, 1.f, 3840.f);
			ImGui::Checkbox("FontAllowUserScaling", &io.FontAllowUserScaling);
			ImGui::Checkbox("ConfigDockingWithShift", &io.ConfigDockingWithShift);
			ImGui::Checkbox("WantCaptureMouse", &io.WantCaptureMouse);
			ImGui::SliderFloat("Mainviewport DpiScale", &ImGui::GetMainViewport()->DpiScale, 1.f, 300.f);

			if (ImGui::TreeNode("Window options"))
			{
				ImGui::Text("These options only affect the parent 'ImGui options' window");
				struct ImGuiWindowOption
				{
					const char *displayName;
					const int ImGuiFlag;
					bool set;
				};
				static std::array<ImGuiWindowOption, 24> ImGuiWindowOptions =
					{{{"NoTitleBar", ImGuiWindowFlags_NoTitleBar, false},
					  {"NoResize", ImGuiWindowFlags_NoResize, false},
					  {"NoMove", ImGuiWindowFlags_NoMove, false},
					  {"NoScrollbar", ImGuiWindowFlags_NoScrollbar, false},
					  {"NoScrollWithMouse", ImGuiWindowFlags_NoScrollWithMouse, false},
					  {"NoCollapse  ", ImGuiWindowFlags_NoCollapse, false},
					  {"AlwaysAutoResize", ImGuiWindowFlags_AlwaysAutoResize, false},
					  {"NoBackground", ImGuiWindowFlags_NoBackground, false},
					  {"NoSavedSettings", ImGuiWindowFlags_NoSavedSettings, false},
					  {"NoMouseInputs", ImGuiWindowFlags_NoMouseInputs, false},
					  {"MenuBar", ImGuiWindowFlags_MenuBar, false},
					  {"HorizontalScrollbar", ImGuiWindowFlags_HorizontalScrollbar, false},
					  {"NoFocusOnAppearing", ImGuiWindowFlags_NoFocusOnAppearing, false},
					  {"NoBringToFrontOnFocus", ImGuiWindowFlags_NoBringToFrontOnFocus, false},
					  {"AlwaysVerticalScrollbar", ImGuiWindowFlags_AlwaysVerticalScrollbar, false},
					  {"AlwaysHorizontalScrollbar", ImGuiWindowFlags_AlwaysHorizontalScrollbar, false},
					  {"AlwaysUseWindowPadding", ImGuiWindowFlags_AlwaysUseWindowPadding, false},
					  {"NoNavInputs", ImGuiWindowFlags_NoNavInputs, false},
					  {"NoNavFocus", ImGuiWindowFlags_NoNavFocus, false},
					  {"UnsavedDocument", ImGuiWindowFlags_UnsavedDocument, false},
					  {"NoDocking", ImGuiWindowFlags_NoDocking, false},
					  {"NoNav", ImGuiWindowFlags_NoNav, false},
					  {"NoDecoration", ImGuiWindowFlags_NoDecoration, false},
					  {"NoInputs", ImGuiWindowFlags_NoInputs, false}}};

				for (int i = 0; i < ImGuiWindowOptions.size(); i++)
				{

					ImGuiWindowOptions[i].set = ((window_flags & ImGuiWindowOptions[i].ImGuiFlag) == ImGuiWindowOptions[i].ImGuiFlag);

					if (ImGui::Checkbox(ImGuiWindowOptions[i].displayName, &ImGuiWindowOptions[i].set))
					{
						if (ImGuiWindowOptions[i].set)
						{
							// Disabling NoMouseInputs as it results in being locked out of ImGui navigation
							if (((ImGuiWindowOptions[i].ImGuiFlag) & ImGuiWindowFlags_NoMouseInputs) == ImGuiWindowFlags_NoMouseInputs)
							{
								ImGuiWindowOptions[i].set = false;
								continue;
							}

							window_flags |= ImGuiWindowOptions[i].ImGuiFlag;
						}
						else
							window_flags &= ~ImGuiWindowOptions[i].ImGuiFlag;
					}
					if (ImGuiWindowOptions[i].ImGuiFlag == ImGuiWindowFlags_NoNav
					|| ImGuiWindowOptions[i].ImGuiFlag == ImGuiWindowFlags_NoDecoration
					|| ImGuiWindowOptions[i].ImGuiFlag == ImGuiWindowFlags_NoInputs)
					{
						ImGui::SameLine();
						ImGui::Text(" (group action)");
					}
				}
				ImGui::TreePop();
			}
		}
		ImGui::End();

		if (ImGui::Begin("Entity draw options"))
		{
			size_t count = 0;

			mDrawCalls.ModifyForEach([&](DrawCall& pDrawCall)
									 {
			count++;
			const std::string title = "Draw call option " + std::to_string(count);

			if(ImGui::TreeNode(title.c_str()))
			{
				ImGui::SliderFloat3("Position", &pDrawCall.mPosition.x, -50.f, 50.f);
				ImGui::SliderFloat3("Rotation", &pDrawCall.mRotation.x, -360.f, 360.f);
				ImGui::SliderFloat3("Scale", &pDrawCall.mScale.x, 0.1f, 10.f);

				{ // Draw mode selection
					if (ImGui::BeginCombo("Draw Mode", convert(pDrawCall.mDrawMode).c_str(), ImGuiComboFlags()))
					{
						for (size_t i = 0; i < drawModes.size(); i++)
						{
							if (ImGui::Selectable(drawModes[i].c_str()))
								pDrawCall.mDrawMode = static_cast<DrawMode>(i);
						}
						ImGui::EndCombo();
					}
				}

				{ // Draw style selection
					if (ImGui::BeginCombo("Draw Style", convert(pDrawCall.mDrawStyle).c_str(), ImGuiComboFlags()))
					{
						for (size_t i = 0; i < drawStyles.size(); i++)
						{
							if (ImGui::Selectable(drawStyles[i].c_str()))
								pDrawCall.mDrawStyle = static_cast<DrawStyle>(i);
						}
						ImGui::EndCombo();
					}
				}

				ImGui::Separator();

				switch (pDrawCall.mDrawStyle)
				{
				case DrawStyle::Textured:
				{
					{// Texture 1
						const std::string currentTexture = pDrawCall.mTexture1.has_value() ? mTextureManager.getTextureName(pDrawCall.mTexture1.value()) : "Empty";
						if (ImGui::BeginCombo("Texture", currentTexture.c_str(), ImGuiComboFlags()))
						{
							mTextureManager.ForEach([&](const Texture &texture)
							{
								if (ImGui::Selectable(texture.mName.c_str()))
								{
									pDrawCall.mTexture1 = texture.getID();
								}
							});
							ImGui::EndCombo();
						}
					}
					if (pDrawCall.mTexture1.has_value())
					{ // Texture 2
						const std::string currentTexture = pDrawCall.mTexture2.has_value() ? mTextureManager.getTextureName(pDrawCall.mTexture2.value()) : "Empty";
						if (ImGui::BeginCombo("Texture 2", currentTexture.c_str(), ImGuiComboFlags()))
						{
							if	(pDrawCall.mTexture2.has_value())
								if (ImGui::Selectable("Empty"))
									pDrawCall.mTexture2 = std::nullopt;

							mTextureManager.ForEach([&](const Texture &texture)
							{
								if (ImGui::Selectable(texture.mName.c_str()))
								{
									pDrawCall.mTexture2 = texture.getID();
								}
							});
							ImGui::EndCombo();
						}
					}
					if (pDrawCall.mTexture1.has_value() && pDrawCall.mTexture2.has_value())
					{ // Only displayed if we have two texture slots set
						if (!pDrawCall.mMixFactor.has_value())
							pDrawCall.mMixFactor = 0.5f;

						ImGui::SliderFloat("Texture mix factor", &pDrawCall.mMixFactor.value(), 0.f, 1.f);
					}
				}
				break;
				case DrawStyle::UniformColour:
				{
					if (!pDrawCall.mColour.has_value())
						pDrawCall.mColour = glm::vec3(1.f, 1.f, 1.f);

					ImGui::ColorEdit3("Colour",  &pDrawCall.mColour.value().x);
				}
				break;
				case DrawStyle::LightMap:
				{
					ImGui::Text("Available texture slots");
					{
						const std::string currentTexture = pDrawCall.mDiffuseTextureID.has_value() ? mTextureManager.getTextureName(pDrawCall.mDiffuseTextureID.value()) : "No texture set";
						if (ImGui::BeginCombo("Diffuse", currentTexture.c_str(), ImGuiComboFlags()))
						{
							mTextureManager.ForEach([&](const Texture &texture)
							{
								if (ImGui::Selectable(texture.mName.c_str()))
									pDrawCall.mDiffuseTextureID = texture.getID();
							});
							ImGui::EndCombo();
						}
					}
					{
						const std::string currentTexture = pDrawCall.mSpecularTextureID.has_value() ? mTextureManager.getTextureName(pDrawCall.mSpecularTextureID.value()) : "No texture set";
						if (ImGui::BeginCombo("Specular", currentTexture.c_str(), ImGuiComboFlags()))
						{
							mTextureManager.ForEach([&](const Texture &texture)
							{
								if (ImGui::Selectable(texture.mName.c_str()))
									pDrawCall.mSpecularTextureID = texture.getID();
							});
							ImGui::EndCombo();
						}
					}
					if (!pDrawCall.mShininess.has_value())
						pDrawCall.mShininess = 64.f;
					ImGui::SliderFloat("Shininess", &pDrawCall.mShininess.value(), 0.1f, 128.f);
				}
				default:
					break;
				}


				ImGui::TreePop();
			} });
		}
		ImGui::End();

		mLightManager.renderImGui();
		mOpenGLAPI->renderImGui();
	}

	if (ImGui::Begin("Performance"))
	{
		// This is showing the last frame's RenderTimeTaken since the update has to happen after renderImGuiFrame below.
		// TODO: Add above comment as a help marker.
		ImGui::Text("ImGui render took: %.3fms", 	mImGuiRenderTimeTakenMS);
		ImGui::Text("Render took: %.3fms", 			mDrawTimeTakenMS);
		ImGui::Text("Frame time: %.3f ms", 			mTimeSinceLastDraw);

		ImGui::Separator();
		ImGui::Text("Target FPS:%d", mTargetFPS);
		ImGui::Text("FPS:");

		ImVec4 colour;
		if (mCurrentFPS >= mTargetFPS * 0.99f)
			colour = ImVec4(0.0f, 1.0f, 0.0f, 1.0f);
		else if (mCurrentFPS <= mTargetFPS * 0.5f)
			colour = ImVec4(1.0f, 0.0f, 0.0f, 1.0f);
		else
			colour = ImVec4(1.0f, 1.0f, 0.0f, 1.0f);
		ImGui::SameLine();
		ImGui::TextColored(colour, "%.0f\t", mCurrentFPS);
		ImGui::SameLine();
		ImGui::Checkbox("Show plot", &mShowFPSPlot);
		if (mShowFPSPlot)
		{
			plotFPSTimes();

			// When changing mFPSSampleSize we have to clear the excess FPS entries in the start of the vector.
			if (ImGui::SliderInt("FPS frame sample size", &mFPSSampleSize, 1, 1000))
				if (mFPSSampleSize < mFPSTimes.size())
					mFPSTimes.erase(mFPSTimes.begin(), mFPSTimes.end() - mFPSSampleSize); // O(n) mFPSTimes.erase linear with mFPSSampleSize
		}

		if(ImGui::TreeNode("Options"))
		{
			ImGui::Checkbox("Render ImGui", &mRenderImGui);
			ImGui::Checkbox("Use raw data", &mUseRawPerformanceData); // Whether we use smoothing for the incoming values of mCurrentFPS.
			if (!mUseRawPerformanceData)
			{
				ImGui::SameLine();
				ImGui::SliderFloat("FPS smoothing factor", &mDataSmoothingFactor, 0.f, 1.f);
			}

			ImGui::TreePop();
		}
	}
	ImGui::End();

	mOpenGLAPI->renderImGuiFrame();
	mImGuiRenderTimeTakenMS = stopWatch.getTime<std::milli, float>();
}

void Renderer::plotFPSTimes()
{
	// We keep a list of mFPSTimes sampling the mCurrentFPS at every Renderer::Draw.
	// mAverageFPS gives the average FPS in the mFPSSampleSize of mFPSTimes.
	// When mFPSTimes is full, clears the first entry to allow ring buffer style push_back for output using ImGui::PlotLines
	if (mFPSTimes.size() <= mFPSSampleSize)
		mFPSTimes.push_back(mCurrentFPS);
	else
	{
		mFPSTimes.erase(mFPSTimes.begin()); // O(n) mFPSTimes.erase linear with mFPSTimes.size()
		mFPSTimes.push_back(mCurrentFPS);
	}
	mAverageFPS = std::reduce(mFPSTimes.begin(), mFPSTimes.end()) / static_cast<float>(mFPSTimes.size()); // O(?) reduce faster than accumulate, per frame
	ImGui::PlotLines("", &mFPSTimes[0], static_cast<int>(mFPSTimes.size()), 0, ("Avg:" + std::to_string(std::round(mAverageFPS))).c_str(), 0.0f, mTargetFPS * 1.25f, ImVec2(ImGui::GetWindowWidth(), mTargetFPS * 1.25f));
}