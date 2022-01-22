#pragma once

#include "InputAPI.hpp"

typedef struct GLFWwindow GLFWwindow;

// Implements InputAPI using GLFW.
// GLFWInput requires a valid GLFW context to be initialised before and a GLFWwindow* instance to register callbacks.
class GLFWInput : public InputAPI
{
public:
    GLFWInput(std::function<void(const Key&)> pOnKeyPressCallback, std::function<void(const float&, const float&)> pOnMouseMoveCallback);

    void pollEvents() override;
    bool closeRequested() override;

private:
    bool mCloseRequested;


    // The remaining functions are required to be static by GLFW for callback
    // ------------------------------------------------------------------------------------------------------------------------------
    static inline GLFWInput* currentActiveInputHandler = nullptr; // The instance of GLFWInput called in the static GLFW callbacks.
    static void windowCloseRequestCallback(GLFWwindow* pWindow); // Called when the window title bar X is pressed.
    static void keyCallback(GLFWwindow* pWindow, int pKey, int pScancode, int pAction, int pMode); // Called when a key is pressed in glfwPollEvents.

    static inline double mLastXPosition = -1.0;
    static inline double mLastYPosition = -1.0;
    static void mouseMoveCallback(GLFWwindow* pWindow, double pNewXPosition, double pNewYPosition);
    static InputAPI::Key convert(const int& pKeyInput);
};