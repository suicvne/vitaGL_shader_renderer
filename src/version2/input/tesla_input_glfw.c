#include "tesla_input.h"

#include <memory.h>
#include <assert.h>
#include <GLFW/glfw3.h>

#define SELF TeslaKeyboardInput* context

typedef struct _TeslaKdbPrivate {
    // Nothing yet.
    GLFWwindow* WindowListening;
    uint32_t    keys[GLFW_KEY_LAST];
    uint32_t    lastKeys[GLFW_KEY_LAST];

} TeslaKdbPrivate;

int Tkbd_IsKeyHeld_glfw(SELF, uint32_t key);
int Tkbd_IsKeyUp_glfw(SELF, uint32_t key);
int TKdb_IsKeyDown_glfw(SELF, uint32_t key);
void TKdb_DestroySelf_glfw(SELF);
void TKdb_PollInput_glfw(SELF);
int TKdb_InitBackend_glfw(SELF, GLFWwindow* existing);


TeslaKeyboardInput TKbd_Create() {
    TeslaKeyboardInput kdbInput = {
        .PrivateData =  malloc(sizeof(TeslaKdbPrivate)),
        .DestroySelf =  TKdb_DestroySelf_glfw,
        .Log =          TInput_Log,
        .PollInput =    TKdb_PollInput_glfw,
        .IsKeyDown =    TKdb_IsKeyDown_glfw,
        .IsKeyUp =      Tkbd_IsKeyUp_glfw,
        .IsKeyHeld =    Tkbd_IsKeyHeld_glfw,
        .InitBackend =  TKdb_InitBackend_glfw,
    };

    /*
    kdbInput.PrivateData->keys = malloc(sizeof(uint32_t) * GLFW_KEY_LAST);
    kdbInput.PrivateData->lastKeys = malloc(sizeof(uint32_t) * GLFW_KEY_LAST);
    */

    for(int i = 0; i < GLFW_KEY_LAST; i++) {
        kdbInput.PrivateData->keys[i] = GLFW_RELEASE;
        kdbInput.PrivateData->lastKeys[i] = GLFW_RELEASE;
    }

    return kdbInput;
}

// UGH
static TeslaKeyboardInput* Tkdb_private_LastKbdCtx = NULL;

void Tkdb_KeyCallback_private(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if(Tkdb_private_LastKbdCtx == NULL) {
        TInput_Log(NULL, "WARNING: Last Kbd Context is null.");
        return;
    }

    // Check if already pressed state.

    if(((action == GLFW_PRESS) || (action == GLFW_REPEAT)) && 
        Tkdb_private_LastKbdCtx->PrivateData->keys[key] == GLFW_PRESS) {
        // TInput_Log(NULL, "Repeat");
        Tkdb_private_LastKbdCtx->PrivateData->keys[key] = GLFW_REPEAT;
    }
    else { 
        // TInput_Log(NULL, "ACTION SET");
        
    }

    Tkdb_private_LastKbdCtx->PrivateData->keys[key] = action;
}

int TKdb_InitBackend_glfw(SELF, GLFWwindow* existing) {
    if(existing != NULL) {
        context->PrivateData->WindowListening = existing;
        glfwSetKeyCallback(existing, Tkdb_KeyCallback_private);
        context->Log(context, "Using existing GLFWwindow (%08p) for TKdb_InitBackend");
        return 0;
    }
    else {
        context->Log(context, "TODO: Init GLFW!");
    }

    return -1;
}

void TKdb_DestroySelf_glfw(SELF) {
    if(context->PrivateData != NULL)
        free(context->PrivateData);
}

void TKdb_PollInput_glfw(SELF) {
    if(context->PrivateData->WindowListening
         != NULL) {
        glfwPollEvents();
        Tkdb_private_LastKbdCtx = context;

        // Check for repeat keys
        for(int i = 0; i < GLFW_KEY_LAST; i++) {


            if(Tkdb_private_LastKbdCtx->PrivateData->lastKeys[i] == GLFW_PRESS && 
                Tkdb_private_LastKbdCtx->PrivateData->keys[i] == GLFW_PRESS)
            {
                // Repeating.
                Tkdb_private_LastKbdCtx->PrivateData->keys[i] = GLFW_REPEAT;
            }
        }

        // Set last keys
        // context->Log(context, "Copy Size: %u", sizeof(uint32_t) * GLFW_KEY_LAST);
        for(int i = 0; i < GLFW_KEY_LAST; i++) {
            context->PrivateData->lastKeys[i] = context->PrivateData->keys[i];
        }
        // LastKeys_p = pCur;
        // memcpy(LastKeys_p, Tkdb_private_LastKbdCtx->PrivateData->keys, sizeof(uint32_t) * GLFW_KEY_LAST);
    }
}

int TKdb_IsKeyDown_glfw(SELF, uint32_t key) {
    int val = (context->PrivateData->keys[key]);
    // context->Log(context, "ugh %d", val);

    return val == GLFW_PRESS;
}

int Tkbd_IsKeyHeld_glfw(SELF, uint32_t key) {
    return context->PrivateData->keys[key] == GLFW_REPEAT;
}

int Tkbd_IsKeyUp_glfw(SELF, uint32_t key) {
    return context->PrivateData->keys[key] == GLFW_RELEASE;
}

#undef SELF

// PollInput -> glfwPollEvents();