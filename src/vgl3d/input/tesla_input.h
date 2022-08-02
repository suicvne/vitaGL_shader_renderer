#ifndef __TESLA_INPUT_H__
#define __TESLA_INPUT_H__

#include <assert.h>

/**
 * @file tesla_input.h
 * @author Michael Santiago (admin@ignoresolutions.xyz)
 * @brief  Header file describing components of an Input interface.
 * 
 * Defines: InputBaseClass containing standard input module functions.
 * 
 * 
 * @version 0.1
 * @date 2022-07-31
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#include "../common.h"

/**
 * @brief Project Tesla Gamepad Input API.
 * Convenience API for wrapping input from various consoles.
 */

/**
 * @brief Defines an enumeration for default gamepad buttons.
 */
typedef enum {
    
    SELECT =        0x00000001,
    L_STICK_IN =    0x00000002,
    R_STICK_IN =    0x00000004,
    START =         0x00000008,

    DPAD_UP =       0x00000010,
    DPAD_RIGHT =    0x00000020,
    DPAD_DOWN =     0x00000040,
    DPAD_LEFT =     0x00000080,

    L_TRIGGER =     0x00000100,
    R_TRIGGER =     0x00000200,
    L_BUMPER =      0x00000400,
    R_BUMPER =      0x00000800,

    /**
     * @brief North face button. 
     * Triangle on Sony.
     * Y on Xbox.
     * X on Nintendo.
     */
    FACE_NORTH =    0x00001000,

    /**
     * @brief West face button.
     * Circle on Sony.
     * B on Xbox.
     * A on Nintendo.
     */
    FACE_EAST =     0x00002000,

    /**
     * @brief South face button.
     * X on Sony.
     * A on Xbox.
     * B on Nintendo.
     */
    FACE_SOUTH =    0x00004000,

    /**
     * @brief East face button.
     * Square on Sony.
     * X on Xbox.
     * Y on Nintendo.
     */
    FACE_WEST =     0x00008000,

} TeslaButtonType;

typedef enum {

    INPUT_TYPE_KEYBOARD,
    INPUT_TYPE_MOUSE,
    INPUT_TYPE_TOUCH,
    INPUT_TYPE_GAMEPAD

} InputType;

// Safe to leave var name off of these.
#define SELF struct _InputBaseClass*
// Defines the function INTERFACE.
// Basically, the gimbal "class"
typedef struct _InputBaseClass {

    int     (*InitBackend)(SELF);
    void    (*DestroySelf)(SELF);
    void    (*Log)(SELF, const char*, ...);
    void    (*PollInput)(SELF);
    int     (*IsBtnDown)(SELF, uint32_t);
    int     (*IsBtnUp)(SELF, uint32_t);
    int     (*IsBtnHeld)(SELF, uint32_t);

} InputBaseClass;

CREATE_LOG_FN(SELF context, TInput, "TeslaInput");

#undef SELF
#define SELF struct _TeslaGamepad* context
struct _TeslaGamepadPrivate;

/**
 * @brief TeslaGamepadInput is a fancy vtable.
 * These are your API functions! Call them on this structure to make your life easy.
 * 
 * These functions can also be overriden per platform.
 * 
 * This structure is a union of InputBaseClass and its own function pointers.
 */
typedef struct _TeslaGamepad {
union {
    InputBaseClass pBase;
    struct {
        int     (*InitBackend)(SELF);
        void    (*DestroySelf)(SELF);
        void    (*Log)(SELF, const char* fmt, ...);
        void    (*PollInput)(SELF);
        int     (*IsButtonDown)(SELF, uint32_t btn);
        int     (*IsButtonUp)(SELF, uint32_t btn);
        int     (*IsButtonHeld)(SELF, uint32_t btn);
    };
};
    struct _TeslaGamepadPrivate *PrivateData;
} TeslaGamepadInput;

#undef SELF

#ifndef VITA
struct GLFWwindow;
#endif

#undef SELF
#define SELF struct _TeslaKeyboard* context

typedef struct _TeslaKeyboard {
union {
    InputBaseClass   pBase;
    struct {
#ifndef VITA
        int         (*InitBackend)(SELF, struct GLFWwindow* window);
#else // Default signature.
        int         (*InitBackend)(SELF);
#endif
        void        (*DestroySelf)(SELF);
        void        (*Log)(SELF, const char* fmt, ...);
        void        (*PollInput)(SELF);
        int         (*IsKeyDown)(SELF, uint32_t key);
        int         (*IsKeyUp)(SELF, uint32_t key);
        int         (*IsKeyHeld)(SELF, uint32_t key);
    };
};
struct _TeslaKdbPrivate *PrivateData;
} TeslaKeyboardInput;

#undef SELF
#define SELF struct _TeslaMouse* context

typedef struct _TeslaMouse {
union {
    InputBaseClass pBase;
    struct {
        int     (*InitBackend)(SELF, struct GLFWwindow* existing);
        void    (*DestroySelf)(SELF);
        void    (*Log)(SELF, const char* fmt, ...);
        void    (*PollInput)(SELF);
        int     (*IsButtonDown)(SELF, uint32_t btn);
        int     (*IsButtonUp)(SELF, uint32_t btn);
        int     (*IsButtonHeld)(SELF, uint32_t btn);
    };
};
    struct _TeslaMousePrivate* PrivateData;
} TestMouse;

#undef SELF
#define SELF struct _TeslaTouch* context

typedef struct _TeslaTouch {
union {
    InputBaseClass pBase;
    struct {
        int     (*InitBackend)(SELF);
        void    (*DestroySelf)(SELF);
        void    (*Log)(SELF, const char* fmt, ...);
        void    (*PollInput)(SELF);
        int     (*IsFingerDown)(SELF, uint32_t finger);
        int     (*IsFingerUp)(SELF, uint32_t finger);
        int     (*IsFingerHeld)(SELF, uint32_t finger);
    };
};
} TestTouch;

/* ========== Default Constructor Functions ============== */
TeslaGamepadInput    TInput_Create();
TeslaKeyboardInput   TKbd_Create();
TestMouse            TMouse_Create();
/* ========== Default Constructor Functions ============== */

/**
 * @brief Tesla Input Interface.
 * 
 * Defines a polymorphic union of the main Tesla Input types:
 *  - TeslaKeyboardInput
 *  - TestMouse
 *  - TestTouch
 *  - TeslaGamepadInput
 * 
 * Following this union is an enum defining the type of input this supports.
 * 
 */
typedef struct _Input {
    union {
        InputBaseClass               pBase;
        TeslaKeyboardInput           pKeyboard;
        TestMouse                    pMouse;
        TestTouch                    pTouch;
        TeslaGamepadInput            pGamepad;
    };
    InputType inputType;
} Input;

static inline Input CommonInput_Create(InputType inputType) {

    Input newInput; // Declare union struct
    newInput.inputType = inputType; // Set union struct type.

    int fail = 0;
    // Variable constructor.
    switch(inputType) {
        case INPUT_TYPE_GAMEPAD:
            newInput.pGamepad   = TInput_Create();
            break;
        case INPUT_TYPE_KEYBOARD:
            newInput.pKeyboard  = TKbd_Create();
            break;
        case INPUT_TYPE_MOUSE:
            newInput.pMouse     = TMouse_Create();
            break;
        case INPUT_TYPE_TOUCH:
        default:
            fail = 1;
            break;
    }

    // Set base log function for convenience.
    newInput.pBase.Log = TInput_Log;

    if(fail != 0) {
        newInput.pBase.Log(&newInput.pBase, "Not supported yet: %d", inputType);
    }

    return newInput;
}

static inline Input* CommonInput_CreateHeap(InputType inputType) {
    Input* newInput = malloc(sizeof(Input));
    *newInput = CommonInput_Create(inputType);

    return newInput;
}


#undef SELF

#endif // __TESLA_INPUT_H__