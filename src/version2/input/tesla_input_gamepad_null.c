#include "tesla_input.h"

#define SELF TeslaGamepadInput* pSelf

int NullInput_InitBackend(SELF)                 { return 0; }
void NullInput_DestroySelf(SELF)                {           }
int NullInput_IsButtonDown(SELF, uint32_t b)    { return 0; }
int NullInput_IsButtonUp(SELF, uint32_t b)      { return 0; }
int NullInput_IsButtonHeld(SELF, uint32_t b)    { return 0; }
void NullInput_PollInput(SELF)                  {           }


TeslaGamepadInput TInput_Create() {
    return (TeslaGamepadInput) {
        .InitBackend =  NullInput_InitBackend,
        .DestroySelf =  NullInput_DestroySelf,
        .pBase.Log =    TInput_Log,
        .IsButtonDown = NullInput_IsButtonDown,
        .IsButtonHeld = NullInput_IsButtonDown,
        .IsButtonUp   = NullInput_IsButtonDown,
        .PollInput =    NullInput_PollInput,
        .PrivateData =  NULL
    };
}