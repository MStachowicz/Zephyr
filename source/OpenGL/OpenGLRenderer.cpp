#include "OpenGLRenderer.hpp"

// ECS
#include "Storage.hpp"

// COMPONENTS
#include "DirectionalLight.hpp"
#include "PointLight.hpp"
#include "SpotLight.hpp"
#include "Texture.hpp"
#include "Collider.hpp"
#include "Camera.hpp"
#include "Mesh.hpp"
#include "Transform.hpp"

// SYSTEMS
#include "MeshSystem.hpp"
#include "TextureSystem.hpp"
#include "SceneSystem.hpp"

// GEOMETRY
#include "Ray.hpp"
#include "Triangle.hpp"

// UTILITY
#include "Logger.hpp"
#include "Utility.hpp"

// PLATFORM
#include "Core.hpp"
#include "Window.hpp"

//GLM
#include "glm/ext/matrix_transform.hpp" // perspective, translate, rotate
#include "glm/gtc/type_ptr.hpp"
#include "glm/gtx/quaternion.hpp"
#include "glm/gtx/string_cast.hpp"

// STD
#include <algorithm>

#include "glad/gl.h"

namespace OpenGL
{
    OpenGLRenderer::DebugOptions::DebugOptions()
        : mShowLightPositions{false}
        , mVisualiseNormals{false}
        , mForceClearColour{false}
        , mClearColour{0.f, 0.f, 0.f, 0.f}
        , mForceDepthTestType{false}
        , mForcedDepthTestType{DepthTestType::Less}
        , mForceBlendType{false}
        , mForcedSourceBlendType{BlendFactorType::SourceAlpha}
        , mForcedDestinationBlendType{BlendFactorType::OneMinusSourceAlpha}
        , mForceCullFacesType{false}
        , mForcedCullFacesType{CullFacesType::Back}
        , mForceFrontFaceOrientationType{false}
        , mForcedFrontFaceOrientationType{FrontFaceOrientation::CounterClockwise}
        , mShowOrientations{false}
        , mShowBoundingBoxes{false}
        , mFillBoundingBoxes{false}
        , mShowCollisionGeometry{false}
        , mCylinders{}
        , mSpheres{}
        , mDepthViewerShader{"depthView"}
        , mVisualiseNormalShader{"visualiseNormal"}
        , mCollisionGeometryShader{"collisionGeometry"}
        , mDebugPoints{}
        , mDebugPointsVAO{}
        , mDebugPointsVBO{}
    {}
    OpenGLRenderer::ViewInformation::ViewInformation()
        : mView{glm::identity<glm::mat4>()}
        , mViewPosition{0.f}
        , mProjection{glm::identity<glm::mat4>()}
    {}

    OpenGLRenderer::OpenGLRenderer(Platform::Window& p_window, System::SceneSystem& pSceneSystem, System::MeshSystem& pMeshSystem, System::TextureSystem& pTextureSystem)
        : m_window{p_window}
        , mScreenFramebuffer{}
        , mSceneSystem{pSceneSystem}
        , mMeshSystem{pMeshSystem}
        , pointLightDrawCount{0}
        , spotLightDrawCount{0}
        , directionalLightDrawCount{0}
        , mPostProcessingOptions{}
        , mUniformColourShader{"uniformColour"}
        , mTextureShader{"texture1"}
        , mScreenTextureShader{"screenTexture"}
        , mSkyBoxShader{"skybox"}
        , mViewInformation{}
        , mDebugOptions{}
    {
        const auto windowSize = m_window.size();
        mScreenFramebuffer.attachColourBuffer(windowSize.x, windowSize.y);
        mScreenFramebuffer.attachDepthBuffer(windowSize.x, windowSize.y);
        set_viewport(0, 0, windowSize.x, windowSize.y);

        LOG("Constructed new OpenGLRenderer instance");
    }

    void OpenGLRenderer::draw(const Data::Model& pModel)
    {
        draw(pModel.mCompositeMesh);
    }
    void OpenGLRenderer::draw(const Data::CompositeMesh& pComposite)
    {
        for (const auto& mesh : pComposite.mMeshes)
            draw(mesh);

        // Recursively draw all the child composites.
        for (const auto& childComposite : pComposite.mChildMeshes)
            draw(childComposite);
    }
    void OpenGLRenderer::draw(const Data::Mesh& pMesh)
    {
        const auto& GLMeshData = pMesh.mGLData;
        GLMeshData.mVAO.bind();

        if (GLMeshData.mEBO.has_value()) // EBO available means drawing with indices.
            draw_elements(PrimitiveMode::Triangles, GLMeshData.mDrawSize);
        else
            draw_arrays(PrimitiveMode::Triangles, 0, GLMeshData.mDrawSize);
    }

    void OpenGLRenderer::start_frame()
    {
        { // Prepare mScreenFramebuffer for rendering
            const auto window_size = m_window.size();
            mScreenFramebuffer.resize(window_size.x, window_size.y);
            set_viewport(0, 0, window_size.x, window_size.y);
            mScreenFramebuffer.bind();
            mScreenFramebuffer.clearBuffers();
            ASSERT(mScreenFramebuffer.isComplete(), "Screen framebuffer not complete, have you attached a colour or depth buffer to it?");
        }

        { // Set global shader uniforms.
            mSceneSystem.getCurrentScene().foreach([this](Component::Camera& p_camera, Component::Transform& p_transform)
            {
                if (p_camera.m_primary)
                {
                    mViewInformation.mViewPosition = p_transform.mPosition;
                    mViewInformation.mView         = p_camera.get_view(p_transform.mPosition);// glm::lookAt(p_transform.mPosition, p_transform.mPosition + p_transform.mDirection, camera_up);
                    mViewInformation.mProjection   = glm::perspective(glm::radians(p_camera.m_FOV), m_window.aspect_ratio(), p_camera.m_near, p_camera.m_far);

                    Shader::set_block_uniform("ViewProperties.view", mViewInformation.mView);
                    Shader::set_block_uniform("ViewProperties.projection", mViewInformation.mProjection);
                }
            });
        }
    }

    void OpenGLRenderer::draw()
    {
        { // Setup the GL state for rendering the scene, the default setup can be overriden by the Debug options struct
            set_polygon_mode(PolygonMode::Fill);

            if (mDebugOptions.mForceClearColour)
                set_clear_colour(mDebugOptions.mClearColour);
            else
                set_clear_colour(glm::vec4(0.f));

            toggle_cull_face(true);
            if (mDebugOptions.mForceCullFacesType)
                set_cull_face_type(mDebugOptions.mForcedCullFacesType);
            else
                set_cull_face_type(CullFacesType::Back);

            if (mDebugOptions.mForceFrontFaceOrientationType)
                set_front_face_orientation(mDebugOptions.mForcedFrontFaceOrientationType);
            else
                set_front_face_orientation(FrontFaceOrientation::CounterClockwise);

            set_depth_test(true);
            if (mDebugOptions.mForceDepthTestType)
                set_depth_test_type(mDebugOptions.mForcedDepthTestType);
            else
                set_depth_test_type(DepthTestType::Less);

            toggle_blending(true);
            if (mDebugOptions.mForceBlendType)
                set_blend_func(mDebugOptions.mForcedSourceBlendType, mDebugOptions.mForcedDestinationBlendType);
            else
                set_blend_func(BlendFactorType::SourceAlpha, BlendFactorType::OneMinusSourceAlpha);
        }

        auto& scene = mSceneSystem.getCurrentScene();
        scene.foreach([&](ECS::Entity& pEntity, Component::Transform& pTransform, Component::Mesh& pMesh )
        {
            if (scene.hasComponents<Component::Texture>(pEntity))
            {
                auto& texComponent = scene.getComponent<Component::Texture>(pEntity);

                mTextureShader.use();
                mTextureShader.set_uniform("model", pTransform.mModel);

                if (texComponent.mDiffuse.has_value())
                {
                    set_active_texture(0);
                    texComponent.mDiffuse.value()->m_GL_texture.bind();
                }
                //   if (texComponent.mSpecular.has_value())
                //   {
                //       set_active_texture(1);
                //       texComponent.mSpecular.value()->mGLTexture.bind();
                //   }
            }
            else
            {
                mUniformColourShader.use();
                mUniformColourShader.set_uniform("model", pTransform.mModel);
                mUniformColourShader.set_uniform("colour", glm::vec3(0.06f, 0.44f, 0.81f));
            }

            draw(*pMesh.mModel);
        });

        renderDebug();
    }

    void OpenGLRenderer::end_frame()
    {
        { // Draw the colour output to the from mScreenFramebuffer texture to the default FBO
            // Unbind after completing draw to ensure all subsequent actions apply to the default FBO and not mScreenFrameBuffer.
            // Disable depth testing to not cull the screen quad the screen texture will be applied onto.
            FBO::unbind();
            set_depth_test(false);
            toggle_cull_face(false);
            set_polygon_mode(PolygonMode::Fill);
            glClear(GL_COLOR_BUFFER_BIT);

            mScreenTextureShader.use();
            { // PostProcessing setters
                mScreenTextureShader.set_uniform("invertColours", mPostProcessingOptions.mInvertColours);
                mScreenTextureShader.set_uniform("grayScale", mPostProcessingOptions.mGrayScale);
                mScreenTextureShader.set_uniform("sharpen", mPostProcessingOptions.mSharpen);
                mScreenTextureShader.set_uniform("blur", mPostProcessingOptions.mBlur);
                mScreenTextureShader.set_uniform("edgeDetection", mPostProcessingOptions.mEdgeDetection);
                mScreenTextureShader.set_uniform("offset", mPostProcessingOptions.mKernelOffset);
            }

            set_active_texture(0);
            mScreenFramebuffer.bindColourTexture();
            draw(*mMeshSystem.mPlanePrimitive);
        }
    }

    void OpenGLRenderer::drawArrow(const glm::vec3& pOrigin, const glm::vec3& pDirection, const float pLength, const glm::vec3& p_colour /*= glm::vec3(1.f,1.f,1.f)*/)
    {
        // Draw an arrow starting at pOrigin of length pLength point in pOrientation.
        // The body/stem of the arrow is a cylinder, the head/tip is a cone model.
        // We use seperate models for both to preserve the proportions which would be lost if we uniformly scaled an 'arrow mesh'

        static const float lengthToBodyLength  = 0.8f; //= 0.714f; // The proportion of the arrow that is the body.
        static const float lengthToBodyDiameter = 0.1f; // The factor from the length of the arrow to the diameter of the body.
        static const float bodyToHeadDiameter = 2.f; // The factor from the diamater of the body to the diameter of the head.

        // Model constants
        static const float cylinderDimensions = 2.f; // The default cylinder model has x, y and z dimensions in the range = [-1 - 1]
        static const float coneDimensions = 2.f;     // The default cone model has x, y and z dimensions in the range     = [-1 - 1]
        static const glm::vec3 modelDirection{0.f, 1.f, 0.f};                // Unit vec up, cone/cylinder models are alligned up (along y) by default.

        // Find the dimensions using pLength
        const float arrowBodyLength = pLength * lengthToBodyLength;
        const float arrowHeadLength = pLength - arrowBodyLength;
        const float arrowBodyDiameter = pLength * lengthToBodyDiameter;
        const float arrowHeadDiameter = arrowBodyDiameter * bodyToHeadDiameter;
        // The rotation to apply to make the arrow mesh point in pDirection.
        const auto arrowToDirectionRot = glm::mat4_cast(Utility::getRotation(modelDirection, pDirection));

        // CYLINDER/BODY
        const glm::vec3 arrowBodyCenter = pOrigin + (pDirection * (arrowBodyLength / 2.f)); // The center of the cylinder.
        const glm::vec3 arrowBodyScale = glm::vec3(arrowBodyDiameter / cylinderDimensions, arrowBodyLength / cylinderDimensions, arrowBodyDiameter / cylinderDimensions);
        glm::mat4 arrowBodyModel = glm::translate(glm::identity<glm::mat4>(), arrowBodyCenter);
        arrowBodyModel = arrowBodyModel * arrowToDirectionRot;
        arrowBodyModel = glm::scale(arrowBodyModel, arrowBodyScale);

        // CONE/HEAD
        const glm::vec3 arrowHeadPosition = pOrigin + (pDirection * (arrowBodyLength + (arrowHeadLength / 2.f))); // The center of the cone
        const glm::vec3 arrowHeadScale = glm::vec3(arrowHeadDiameter / coneDimensions, arrowHeadLength / coneDimensions, arrowHeadDiameter / coneDimensions);
        glm::mat4 arrowHeadModel = glm::translate(glm::identity<glm::mat4>(), arrowHeadPosition);
        arrowHeadModel = arrowHeadModel * arrowToDirectionRot;
        arrowHeadModel = glm::scale(arrowHeadModel, arrowHeadScale);

        mUniformColourShader.use();
        mUniformColourShader.set_uniform("colour", p_colour);

        mUniformColourShader.set_uniform("model", arrowHeadModel);
        draw(*mMeshSystem.mConePrimitive);

        mUniformColourShader.set_uniform("model", arrowBodyModel);
        draw(*mMeshSystem.mCylinderPrimitive);
    }
    void OpenGLRenderer::drawCylinder(const glm::vec3& pStart, const glm::vec3& pEnd, const float pDiameter, const glm::vec3& p_colour /*= glm::vec3(1.f,1.f,1.f)*/)
    {
        drawCylinder({pStart, pEnd, pDiameter}, p_colour);
    }
    void OpenGLRenderer::drawCylinder(const Geometry::Cylinder& pCylinder, const glm::vec3& p_colour /*=glm::vec3(1.f, 1.f, 1.f)*/)
    {
        static const float cylinderDimensions = 2.f; // The default cylinder model has x, y and z dimensions in the range = [-1 - 1]
        static const glm::vec3 cylinderAxis{0.f, 1.f, 0.f}; // Unit vec up, cone/cylinder models are alligned up (along y) by default.

        const float length = glm::length(pCylinder.mTop - pCylinder.mBase);
        const glm::vec3 direction = glm::normalize(pCylinder.mTop - pCylinder.mBase);
        const glm::vec3 center = pCylinder.mBase + (direction * (length / 2.f)); // The center of the cylinder in world space.
        const glm::mat4 rotation = glm::mat4_cast(glm::rotation(cylinderAxis, direction));
        // Cylinder model is alligned along the y-axis, scale the x and z to diameter and y to length before rotating.
        const glm::vec3 scale = glm::vec3(pCylinder.mDiameter / cylinderDimensions, length / cylinderDimensions, pCylinder.mDiameter / cylinderDimensions);

        glm::mat4 modelMat = glm::translate(glm::identity<glm::mat4>(), center);
        modelMat = modelMat * rotation;
        modelMat = glm::scale(modelMat, scale);

        mUniformColourShader.set_uniform("colour", p_colour);
        mUniformColourShader.set_uniform("model", modelMat);
        draw(*mMeshSystem.mCylinderPrimitive);
    }
    void OpenGLRenderer::drawSphere(const glm::vec3& pCenter, const float& pRadius, const glm::vec3& p_colour/*= glm::vec3(1.f, 1.f, 1.f)*/)
    {
        drawSphere({pCenter, pRadius}, p_colour);
    }
    void OpenGLRenderer::drawSphere(const Geometry::Sphere& pSphere, const glm::vec3& p_colour/*= glm::vec3(1.f, 1.f, 1.f)*/)
    {
        static const float sphereModelRadius = 1.f; // The default sphere model has XYZ dimensions in the range [-1 - 1] = radius of 1.f, diameter 2.f

        glm::mat4 modelMat = glm::translate(glm::identity<glm::mat4>(), pSphere.mCenter);
        modelMat = glm::scale(modelMat, glm::vec3(pSphere.mRadius / sphereModelRadius));

        mUniformColourShader.set_uniform("colour", p_colour);
        mUniformColourShader.set_uniform("model", modelMat);
        draw(*mMeshSystem.mSpherePrimitive);
    }

    void OpenGLRenderer::renderDebug()
    {
        auto& scene = mSceneSystem.getCurrentScene();
        toggle_cull_face(true);

        {// Render all the collision geometry by pushing all the mesh triangles transformed in world-space.
            mDebugOptions.mDebugPoints.clear();
            mDebugOptions.mDebugPointsVAO.bind();
            mDebugOptions.mDebugPointsVBO.clear();

            if (mDebugOptions.mShowCollisionGeometry)
            {
                mSceneSystem.getCurrentScene().foreach([&](Component::Transform& pTransform, Component::Mesh& pMesh)
                {
                    mDebugOptions.mCollisionGeometryShader.use();

                    pMesh.mModel->mCompositeMesh.forEachMesh([this, &pTransform](const Data::Mesh& pMesh)
                    {
                        for (auto triangle : pMesh.mTriangles)
                        {
                            triangle.transform(pTransform.mModel);
                            mDebugOptions.mDebugPoints.insert(mDebugOptions.mDebugPoints.end(), {triangle.m_point_1, triangle.m_point_2, triangle.m_point_3});
                        }
                    });
                });

                if (!mDebugOptions.mDebugPoints.empty())
                {
                    mDebugOptions.mDebugPointsVAO.bind();
                    mDebugOptions.mDebugPointsVBO = VBO();
                    mDebugOptions.mDebugPointsVBO.bind();
                    mDebugOptions.mDebugPointsVBO.setData(mDebugOptions.mDebugPoints, VertexAttribute::Position3D);

                    toggle_cull_face(false); // Disable culling to show exact geometry
                    mDebugOptions.mCollisionGeometryShader.set_uniform("viewPosition", mViewInformation.mViewPosition);
                    mDebugOptions.mCollisionGeometryShader.set_uniform("colour", glm::vec4(0.f, 1.f, 0.f, 0.5f));
                    mDebugOptions.mCollisionGeometryShader.set_uniform("model", glm::mat4(1.f));
                    draw_arrays(PrimitiveMode::Triangles, 0, static_cast<int>(mDebugOptions.mDebugPoints.size()));
                }
            }
        }

        if (mDebugOptions.mShowLightPositions)
        {
            mUniformColourShader.use();

            mSceneSystem.getCurrentScene().foreach([this](Component::PointLight& pPointLight)
            {
                mUniformColourShader.set_uniform("model", Utility::GetModelMatrix(pPointLight.mPosition, glm::vec3(0.f), glm::vec3(0.1f)));
                mUniformColourShader.set_uniform("colour", pPointLight.mColour);
                draw(*mMeshSystem.mCubePrimitive);
            });
        }

        drawArrow(glm::vec3(0.f, 0.f, 0.f), glm::vec3(1.f, 0.f, 0.f), 1.f, glm::vec3(1.f, 0.f, 0.f));
        drawArrow(glm::vec3(0.f, 0.f, 0.f), glm::vec3(0.f, 1.f, 0.f), 1.f, glm::vec3(0.f, 1.f, 0.f));
        drawArrow(glm::vec3(0.f, 0.f, 0.f), glm::vec3(0.f, 0.f, 1.f), 1.f, glm::vec3(0.f, 0.f, 1.f));

        {// Draw debug shapes
            for (const auto& cylinder : mDebugOptions.mCylinders)
                drawCylinder(cylinder);
            for (const auto& sphere : mDebugOptions.mSpheres)
                drawSphere(sphere);
        }

        if (mDebugOptions.mShowBoundingBoxes)
        {
            set_polygon_mode(mDebugOptions.mFillBoundingBoxes ? PolygonMode::Fill : PolygonMode::Line);
            mUniformColourShader.use();

            scene.foreach([&](Component::Transform& pTransform, Component::Mesh& pMesh, Component::Collider& pCollider)
            {
                mUniformColourShader.set_uniform("model", pCollider.getWorldAABBModel());
                mUniformColourShader.set_uniform("colour", pCollider.mCollided ? glm::vec3(1.f, 0.f, 0.f) : glm::vec3(0.f, 1.f, 0.f));
                draw(*mMeshSystem.mCubePrimitive);
            });
        }

        if (mDebugOptions.mShowOrientations)
        {
            scene.foreach([&](Component::Transform& pTransform, Component::Mesh& pMesh)
            {
                auto axes = pTransform.get_local_axes();
                drawArrow(pTransform.mPosition, axes[0], 1.f, glm::vec3(1.f, 0.f, 0.f));
                drawArrow(pTransform.mPosition, axes[1], 1.f, glm::vec3(0.f, 1.f, 0.f));
                drawArrow(pTransform.mPosition, axes[2], 1.f, glm::vec3(0.f, 0.f, 1.f));
            });
        }
    }

    void OpenGLRenderer::setupLights()
    {
        mSceneSystem.getCurrentScene().foreach([this](Component::PointLight& pPointLight)
        {
            setShaderVariables(pPointLight);
        });
        mSceneSystem.getCurrentScene().foreach([this](Component::DirectionalLight& pDirectionalLight)
        {
            setShaderVariables(pDirectionalLight);
        });
        mSceneSystem.getCurrentScene().foreach([this](Component::SpotLight& pSpotLight)
        {
            setShaderVariables(pSpotLight);
        });
    }
    void OpenGLRenderer::setShaderVariables(const Component::PointLight& pPointLight)
    {
        const std::string uniform     = "Lights.mPointLights[" + std::to_string(pointLightDrawCount) + "]";
        const glm::vec3 diffuseColour = pPointLight.mColour * pPointLight.mDiffuseIntensity;
        const glm::vec3 ambientColour = diffuseColour * pPointLight.mAmbientIntensity;

        //Shader::set_block_uniform((uniform + ".position").c_str(), pPointLight.mPosition);
        //Shader::set_block_uniform((uniform + ".ambient").c_str(), ambientColour);
        //Shader::set_block_uniform((uniform + ".diffuse").c_str(), diffuseColour);
        //Shader::set_block_uniform((uniform + ".specular").c_str(), glm::vec3(pPointLight.mSpecularIntensity));
        //Shader::set_block_uniform((uniform + ".constant").c_str(), pPointLight.mConstant);
        //Shader::set_block_uniform((uniform + ".linear").c_str(), pPointLight.mLinear);
        //Shader::set_block_uniform((uniform + ".quadratic").c_str(), pPointLight.mQuadratic);

        pointLightDrawCount++;
    }
    void OpenGLRenderer::setShaderVariables(const Component::DirectionalLight& pDirectionalLight)
    {
        const glm::vec3 diffuseColour = pDirectionalLight.mColour * pDirectionalLight.mDiffuseIntensity;
        const glm::vec3 ambientColour = diffuseColour * pDirectionalLight.mAmbientIntensity;

        //Shader::set_block_uniform("Lights.mDirectionalLight.direction", pDirectionalLight.mDirection);
        //Shader::set_block_uniform("Lights.mDirectionalLight.ambient", ambientColour);
        //Shader::set_block_uniform("Lights.mDirectionalLight.diffuse", diffuseColour);
        //Shader::set_block_uniform("Lights.mDirectionalLight.specular", glm::vec3(pDirectionalLight.mSpecularIntensity));

        directionalLightDrawCount++;
    }
    void OpenGLRenderer::setShaderVariables(const Component::SpotLight& pSpotLight)
    {
        const glm::vec3 diffuseColour = pSpotLight.mColour * pSpotLight.mDiffuseIntensity;
        const glm::vec3 ambientColour = diffuseColour * pSpotLight.mAmbientIntensity;

        //Shader::set_block_uniform("Lights.mSpotLight.position", pSpotLight.mPosition);
        //Shader::set_block_uniform("Lights.mSpotLight.direction", pSpotLight.mDirection);
        //Shader::set_block_uniform("Lights.mSpotLight.diffuse", diffuseColour);
        //Shader::set_block_uniform("Lights.mSpotLight.ambient", ambientColour);
        //Shader::set_block_uniform("Lights.mSpotLight.specular", glm::vec3(pSpotLight.mSpecularIntensity));
        //Shader::set_block_uniform("Lights.mSpotLight.constant", pSpotLight.mConstant);
        //Shader::set_block_uniform("Lights.mSpotLight.linear", pSpotLight.mLinear);
        //Shader::set_block_uniform("Lights.mSpotLight.quadratic", pSpotLight.mQuadratic);
        //Shader::set_block_uniform("Lights.mSpotLight.cutOff", pSpotLight.mCutOff);
        //Shader::set_block_uniform("Lights.mSpotLight.cutOff", pSpotLight.mOuterCutOff);

        spotLightDrawCount++;
    }
} // namespace OpenGL