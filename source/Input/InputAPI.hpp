#pragma once

#include "functional"

// Interface used to communicate with Zephyr::Input.
// Initialise and all subscribe functions must be called before pollEvents
// Input subscribes functions for derived types to callback such as onKeyPress
class InputAPI
{
public:
    enum class Key
    {
        KEY_0, KEY_1, KEY_2, KEY_3, KEY_4, KEY_5, KEY_6, KEY_7, KEY_8, KEY_9,

        KEY_A, KEY_B, KEY_C, KEY_D, KEY_E, KEY_F, KEY_G, KEY_H, KEY_I, KEY_J, KEY_K, KEY_L, KEY_M,
        KEY_N, KEY_O, KEY_P, KEY_Q, KEY_R, KEY_S, KEY_T, KEY_U, KEY_V, KEY_W, KEY_X, KEY_Y, KEY_Z,

        KEY_F1, KEY_F2, KEY_F3, KEY_F4, KEY_F5, KEY_F6, KEY_F7, KEY_F8, KEY_F9, KEY_F10, KEY_F11, KEY_F12,

        KEY_SPACE, KEY_ESCAPE, KEY_ENTER, KEY_TAB,
        KEY_UNKNOWN
    };
    // Provides a callback function for the derived InputAPI to call when a keypress is encountered.
    void subscribeKeyCallback(std::function<void(const Key&)> pOnKeyPressCallback)
    {
        onKeyPress = pOnKeyPressCallback;
    }

    virtual void initialise()       = 0;
    virtual void pollEvents()       = 0;
    virtual bool closeRequested()   = 0;

protected:
    std::function<void(const Key &)> onKeyPress; // The function a derived InputAPI calls
};