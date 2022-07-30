#include "tesla_input.h"
#include <psp2/ctrl.h>

typedef struct _TeslaGamepadPrivate {
    SceCtrlData VitaCtrlData;
    union {
        uint32_t TeslaGamepadBtnBitfield;
        struct {
            uint32_t select     : 1;
            uint32_t l3         : 1;
            uint32_t r3         : 1;
            uint32_t start      : 1;
            uint32_t dpad_up    : 1;
            uint32_t dpad_right : 1;
            uint32_t dpad_down  : 1;
            uint32_t dpad_left  : 1;
            uint32_t l2         : 1;
            uint32_t r2         : 1;
            uint32_t l1         : 1;
            uint32_t r1         : 1;
            uint32_t triangle   : 1;
            uint32_t circle     : 1;
            uint32_t cross      : 1;
            uint32_t square     : 1;
        } Btns;
    };
} TeslaGamepadPrivate;

#define SELF TeslaGamepadInput* context

int     TInput_InitBackend_Vita(SELF);
void    TInput_DestroyBackend_Vita(SELF);
void    TInput_DestroySelf_Vita(SELF);
void    TInput_Poll_Vita(SELF);
int     TInput_IsButtonDown_Vita(SELF);

TeslaGamepadInput TInput_Create() {
    TeslaGamepadInput newInput = (TeslaGamepadInput) {
        .PrivateData =      malloc(sizeof(TeslaGamepadPrivate)),
        .InitBackend =      TInput_InitBackend_Vita,
        .DestroyBackend =   TInput_DestroyBackend_Vita,
        .DestroySelf =      TInput_DestroySelf_Vita,
        .Log =              TInput_Log
    };

    newInput.PrivateData->VitaCtrlData = (SceCtrlData){0};

    return newInput;
}

void TInput_Poll_Vita(SELF) {
    // Poll using Sce functions.
    // Store into bitfield via translation
    sceCtrlPeekBufferPositive(0, 
        &context->PrivateData->VitaCtrlData, 
        1
    );

    // These are directly compatible.
    context->PrivateData->TeslaGamepadBtnBitfield
        = context->PrivateData->VitaCtrlData.buttons;

}

int TInput_IsButtonDown_Vita(SELF, TeslaButtonType btn) {
    return ()
}

int TInput_InitBackend_Vita(SELF) {
    context->Log(context, "Initialized Tesla Gamepad Input for PS Vita");
    sceCtrlSetSamplingMode(SCE_CTRL_MODE_ANALOG);

    return 0;
}

void TInput_DestroyBackend_Vita(SELF) {
    // Nothing to do here. The Sony input backend doesn't need to be shut down.
}

void TInput_DestroySelf_Vita(SELF) {
    // Nothing to do here.
    // We're not allocating anything on the heap...at least I hope not.
}

#undef SELF