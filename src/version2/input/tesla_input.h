#ifndef __TESLA_INPUT_H__
#define __TESLA_INPUT_H__

#include "../common.h"

/**
 * @brief Project Tesla Gamepad Input API.
 * Convenience API for wrapping input from various consoles.
 */

/**
 * @brief Defines an enumeration for default gamepad buttons.
 */
typedef enum {

    DPAD_UP =       0x01,
    DPAD_DOWN =     0x02,
    DPAD_LEFT =     0x03,
    DPAD_RIGHT =    0x04,
    L_BUMPER =      0X05,
    R_BUMPER =      0X06,
    L_TRIGGER =     0X07,
    R_TRIGGER =     0X08,
    SELECT =        0X09,
    START =         0X0A,

    /**
     * @brief North face button. 
     * Triangle on Sony.
     * Y on Xbox.
     * X on Nintendo.
     */
    FACE_NORTH =    0x0B,

    /**
     * @brief South face button.
     * X on Sony.
     * A on Xbox.
     * B on Nintendo.
     */
    FACE_SOUTH =    0x0C,

    /**
     * @brief East face button.
     * Square on Sony.
     * X on Xbox.
     * Y on Nintendo.
     */
    FACE_EAST =     0x0D,

    /**
     * @brief West face button.
     * Circle on Sony.
     * B on Xbox.
     * A on Nintendo.
     */
    FACE_WEST =     0x0E,
    L_STICK_IN =    0x0F,
    R_STICK_IN =    0x10
} TeslaButtonType;

#ifndef SELF
#define SELF struct _TeslaGamepad* context
#endif

struct _TeslaGamepadPrivate;

/**
 * @brief TeslaGamepadInput is a fancy vtable.
 * These are your API functions! Call them on this structure to make your life easy.
 * 
 * These functions can also be overriden per platform.
 */
typedef struct _TeslaGamepad {

    int     (*InitBackend)(SELF);
    void    (*DestroyBackend)(SELF);
    void    (*DestroySelf)(SELF);
    void    (*Log)(SELF, const char* fmt, ...);

    struct _TeslaGamepadPrivate *PrivateData;

} TeslaGamepadInput;


CREATE_LOG_FN(SELF, TInput, "TeslaGamepadInput");
TeslaGamepadInput    TInput_Create();


#undef SELF

#endif // __TESLA_INPUT_H__