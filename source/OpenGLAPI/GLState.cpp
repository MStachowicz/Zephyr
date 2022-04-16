#include "GLState.hpp"

#include "Logger.hpp"

#include "glad/gl.h"
#include "imgui.h"

namespace GLType
{
    int convert(const BlendFactorType& pBlendFactor)
    {
    	switch (pBlendFactor)
    	{
    		case BlendFactorType::Zero: 						return GL_ZERO;
    		case BlendFactorType::One:						    return GL_ONE;
    		case BlendFactorType::SourceColour:				    return GL_SRC_COLOR;
    		case BlendFactorType::OneMinusSourceColour:		    return GL_ONE_MINUS_SRC_COLOR;
    		case BlendFactorType::DestinationColour:			return GL_DST_COLOR;
    		case BlendFactorType::OneMinusDestinationColour:	return GL_ONE_MINUS_DST_COLOR;
    		case BlendFactorType::SourceAlpha:				    return GL_SRC_ALPHA;
    		case BlendFactorType::OneMinusSourceAlpha:		    return GL_ONE_MINUS_SRC_ALPHA;
    		case BlendFactorType::DestinationAlpha:			    return GL_DST_ALPHA;
    		case BlendFactorType::OneMinusDestinationAlpha: 	return GL_ONE_MINUS_DST_ALPHA;
    		case BlendFactorType::ConstantColour:	 		    return GL_CONSTANT_COLOR;
    		case BlendFactorType::OneMinusConstantColour: 	    return GL_ONE_MINUS_CONSTANT_COLOR;
    		case BlendFactorType::ConstantAlpha:		 		return GL_CONSTANT_ALPHA;
    		case BlendFactorType::OneMinusConstantAlpha: 	    return GL_ONE_MINUS_CONSTANT_ALPHA;

    		default: ZEPHYR_ASSERT(false, "Unknown BlendFactorType requested");	return 0;
    	}
    }
}

GLState::GLState()
    : mDepthTest(true)
    , mDepthTestType(GLType::DepthTestType::Less)
    , mBlend(true)
    , mSourceBlendFactor(GLType::BlendFactorType::SourceAlpha)
    , mDestinationBlendFactor(GLType::BlendFactorType::OneMinusSourceAlpha)
    , mBufferClearBitField(GL_COLOR_BUFFER_BIT) // GL_DEPTH_BUFFER_BIT is optionally added in toggleDepthTest()
    , mWindowClearColour{0.0f, 0.0f, 0.0f, 1.0f}
{
    toggleDepthTest(mDepthTest);
    if (mDepthTest)
        setDepthTestType(mDepthTestType);

    toggleBlending(mBlend);
    if (mBlend)
        setBlendFunction(mSourceBlendFactor, mDestinationBlendFactor);
}

void GLState::toggleDepthTest(const bool& pDepthTest)
{
    mDepthTest = pDepthTest;

    if (mDepthTest)
	{
		glEnable(GL_DEPTH_TEST);
		mBufferClearBitField |= GL_DEPTH_BUFFER_BIT;
	}
	else
	{
		glDisable(GL_DEPTH_TEST);
        glClear(GL_DEPTH_BUFFER_BIT); // Clear the buffer before removing it from mBufferClearBitField;
		mBufferClearBitField &= ~GL_DEPTH_BUFFER_BIT;
	}
}
// Pixels can be drawn using a function that blends the incoming (source) RGBA values with the RGBA values that are already in the frame buffer (the destination values).
// Blending is default disabled in OpenGL.
void GLState::toggleBlending(const bool& pBlend)
{
    mBlend = pBlend;

    if (mBlend)
        glEnable(GL_BLEND);
    else
        glDisable(GL_BLEND);
}

void GLState::setDepthTestType(const GLType::DepthTestType& pType)
{
    ZEPHYR_ASSERT(mDepthTest, "Depth test has to be on to allow setting the depth testing type.");

	mDepthTestType = pType;
	switch (mDepthTestType)
	{
	    case GLType::DepthTestType::Always:		  glDepthFunc(GL_ALWAYS); 	return;
	    case GLType::DepthTestType::Never:		  glDepthFunc(GL_NEVER); 	return;
	    case GLType::DepthTestType::Less:		  glDepthFunc(GL_LESS); 	return;
	    case GLType::DepthTestType::Equal:		  glDepthFunc(GL_EQUAL);	return;
	    case GLType::DepthTestType::LessEqual:	  glDepthFunc(GL_LEQUAL	);  return;
	    case GLType::DepthTestType::Greater:	  glDepthFunc(GL_GREATER); 	return;
	    case GLType::DepthTestType::NotEqual:	  glDepthFunc(GL_NOTEQUAL); return;
	    case GLType::DepthTestType::GreaterEqual: glDepthFunc(GL_GEQUAL); 	return;

	    default: ZEPHYR_ASSERT(false, "Unknown DepthTestType requested");	return;
	}
}

void GLState::setBlendFunction(const GLType::BlendFactorType &pSourceFactor, const GLType::BlendFactorType &pDestinationFactor)
{
    GLboolean enabled = glIsEnabled(GL_BLEND);
    ZEPHYR_ASSERT(enabled, "Blending has to be enabled to set blend function.");

    glBlendFunc(convert(pSourceFactor), convert(pDestinationFactor)); // It is also possible to set individual RGBA factors using glBlendFuncSeparate().

    // BlendFactors using constant require glBlendColor() to be called to set the RGBA constant values.
    ZEPHYR_ASSERT((pSourceFactor    != GLType::BlendFactorType::ConstantColour
    && pSourceFactor                != GLType::BlendFactorType::OneMinusConstantColour
    && pSourceFactor                != GLType::BlendFactorType::ConstantAlpha
    && pSourceFactor                != GLType::BlendFactorType::OneMinusConstantAlpha
    && pDestinationFactor           != GLType::BlendFactorType::ConstantColour
    && pDestinationFactor           != GLType::BlendFactorType::OneMinusConstantColour
    && pDestinationFactor           != GLType::BlendFactorType::ConstantAlpha
    && pDestinationFactor           != GLType::BlendFactorType::OneMinusConstantAlpha)
    , "Constant blend factors require glBlendColor() to set the constant. Not supported yet.");
}

void GLState::setClearColour(const std::array<float, 4> &pColour)
{
    mWindowClearColour = pColour;
    glClearColor(mWindowClearColour[0], mWindowClearColour[1], mWindowClearColour[2], mWindowClearColour[3]);
}

void GLState::clearBuffers()
{
    glClear(mBufferClearBitField);
}

 // Outputs the current GLState with options to change flags.
void GLState::renderImGui()
{
    if (ImGui::ColorEdit4("Window clear colour", mWindowClearColour.data()))
        setClearColour(mWindowClearColour);

    { // Depth testing options
        if (ImGui::Checkbox("Depth test", &mDepthTest))
            toggleDepthTest(mDepthTest);

        if (mDepthTest)
        {
            ImGui::SameLine();
            if (ImGui::BeginCombo("Depth test type", GLType::toString(mDepthTestType).c_str(), ImGuiComboFlags()))
            {
                for (size_t i = 0; i < util::toIndex(GLType::DepthTestType::Count); i++)
                {
                    if (ImGui::Selectable(GLType::depthTestTypes[i].c_str()))
                        setDepthTestType(static_cast<GLType::DepthTestType>(i));
                }
                ImGui::EndCombo();
            }
        }
    }

    {// Blending options
        if (ImGui::Checkbox("Blending", &mBlend))
            toggleBlending(mBlend);

        if (mBlend)
        {
            ImGui::Text("Blend function:");
            ImGui::SameLine();

            const float width = ImGui::GetWindowWidth() * 0.25f;
            ImGui::SetNextItemWidth(width);
            if (ImGui::BeginCombo("Source", GLType::toString(mSourceBlendFactor).c_str()))
            {
                for (size_t i = 0; i < util::toIndex(GLType::BlendFactorType::Count); i++)
                {
                    if (ImGui::Selectable(GLType::blendFactorTypes[i].c_str()))
                    {
                        mSourceBlendFactor = static_cast<GLType::BlendFactorType>(i);
                        setBlendFunction(mSourceBlendFactor, mDestinationBlendFactor);
                    }
                }
                ImGui::EndCombo();
            }

            ImGui::SameLine();
            ImGui::SetNextItemWidth(width);
            if (ImGui::BeginCombo("Destination", GLType::toString(mDestinationBlendFactor).c_str(), ImGuiComboFlags()))
            {
                for (size_t i = 0; i < util::toIndex(GLType::BlendFactorType::Count); i++)
                {
                    if (ImGui::Selectable(GLType::blendFactorTypes[i].c_str()))
                    {
                        mDestinationBlendFactor = static_cast<GLType::BlendFactorType>(i);
                        setBlendFunction(mSourceBlendFactor, mDestinationBlendFactor);
                    }
                }
                ImGui::EndCombo();
            }
        }

    }
}