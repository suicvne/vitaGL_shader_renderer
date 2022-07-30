#include "tesla_input.h"
#include <psp2/ctrl.h>

typedef struct _TeslaGamepadPrivate {
    SceCtrlData VitaCtrlData;
} TeslaGamepadPrivate;

#define SELF TeslaGamepadInput* context

int     TInput_InitBackend_Vita(SELF);
void    TInput_DestroyBackend_Vita(SELF);
void    TInput_DestroySelf_Vita(SELF);

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