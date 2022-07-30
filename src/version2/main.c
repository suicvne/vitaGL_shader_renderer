#include <stdio.h>
#include "vgl3d.h"
#include "input/tesla_input.h"

static inline float lerp(float a, float b, float f)
{
    return a + f * (b - a);
}

static float _CurTime = 0.0f;
static uint8_t _OpType = 0;
static int doSpin = 1;

void doUpdate(VGL3DContext* context, float dt)
{
    if(!doSpin) return;
    if(_OpType == 0)
        _CurTime += dt;
    else _CurTime -= dt;

    if(_CurTime > 1.0f || _CurTime < 0.0f)
        _OpType = !_OpType;

    // Manipulate the camera.
    vec3 closePos = {-5.0f, 0.0f, -10.0f};

    vec3 closeRot = { -30.0f, 0.0f, 0.0f };
    vec3 farRot = { -20.0f, -360.f, 20.0f };
    vec3 lerpedRot = { 0 };

    glm_vec3_lerp(closeRot, farRot, _CurTime, lerpedRot);

    // Update camera position.
    context->SetCamera(context, closePos, lerpedRot);
}

#ifdef VITA
#include <debugnet.h>
#ifndef NETDBG_IP_SERVER
#define NETDBG_IP_SERVER "192.168.1.204"
#endif
#ifndef NETDBG_PORT_SERVER
#define NETDBG_PORT_SERVER 18194
#endif
static uint8_t VitaNetLogInitialized_private = 0;
void NetLogForVita_private(VGL3DContext* context, const char *fmt, ...) {
    // Lazy initialized.
    if(!VitaNetLogInitialized_private) {
        debugNetInit(NETDBG_IP_SERVER, NETDBG_PORT_SERVER, DEBUG);
        VitaNetLogInitialized_private = 1;
    }


    char buffer[2048];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buffer, 2048, fmt, args);
    debugNetPrintf(DEBUG, "[VGL3DLog] %s\n", buffer);
    va_end(args);
}

void NetLogForVita_finished_private() {
    if(VitaNetLogInitialized_private) {
        debugNetFinish();
    }
}

#define NETLOG_VITA_FINISH() NetLogForVita_finished_private()
#define VITA_EXAMPLE_TEXTURE "app0:background2-1.png"
#else
// Empty define.
#define NETLOG_VITA_FINISH()
#define VITA_EXAMPLE_TEXTURE "../background2-1.png"
#endif

// TODO: Create method that offsets the model via the MVP.
void DrawCube(VGL3DContext* context, vec3 pos) {
    const float SCALE = 1.0f;
        /** Draw Sides **/
        context->DrawQuad(context, 
            0.f + pos[0], 0.f + pos[1], -1.f + pos[2], 
            (vec3){0.f, 0.f, 0.f}, 
            (vec3){SCALE,SCALE,SCALE}, 
            (vec4){1.0f, 1.0f, 1.0f, 1.0f}
        );
        context->DrawQuad(context, 
            -0.5f + pos[0], 0.f + pos[1], -0.5f + pos[2], 
            (vec3){0.f, 90.f, 0.f}, 
            (vec3){SCALE,SCALE,SCALE},
            (vec4){1.0f, 1.0f, 1.0f, 1.0f}
        );
        
        context->DrawQuad(context, 
            0.f + pos[0], 0.f + pos[1], 0.f + pos[2], 
            (vec3){0.f, 180.f, 0.f}, 
            (vec3){SCALE,SCALE,SCALE},
            (vec4){1.0f, 1.0f, 1.0f, 1.0f}
        );
        context->DrawQuad(context, 
            0.5f + pos[0], 0.f + pos[1], -0.5f + pos[2], 
            (vec3){0.f, 270.f, 0.f}, 
            (vec3){SCALE,SCALE,SCALE},
            (vec4){1.0f, 1.0f, 1.0f, 1.0f}
        );
        /** End Draw Sides **/
        
        // Top
        context->DrawQuad(context, 
            0.f + pos[0], 1.5f + pos[1], -0.5f + pos[2], 
            (vec3){90.f, 0.f, 0.f}, 
            (vec3){SCALE,SCALE,SCALE},
            (vec4){0.0f,0.5f,0.f,1.0f}
        );

        // Bottom
        context->DrawQuad(context, 
            0.f + pos[0], 0.5f + pos[1], -0.5f + pos[2], 
            (vec3){90.f, 0.f, 0.f}, 
            (vec3){SCALE,SCALE,SCALE},
            (vec4){0.0f,1.f,0.f,1.0f}
        );
}

void CheckInput_keyboard(TeslaKeyboardInput* kbdInput) {
    if(kbdInput->IsKeyHeld(kbdInput, GLFW_KEY_SPACE)) {
        doSpin = 1;
        // kbdInput->Log(kbdInput, "Keydown");
        // doSpin = !doSpin;
    }
    else doSpin = 0;
}

int main() {
    printf("Hello world!\n");

    VGL3DContext graphics = VGL3D_Create();


    #ifdef VITA
    // Override Log function for Vita.
    // Redirect logs into debugNet instead of general Vita stdout.
    graphics.Log = NetLogForVita_private;
    #endif

    graphics.InitBackend(&graphics);
    graphics.Log(&graphics, "LOGS TEST! %d", 69);

    VTEX thisTex = graphics.LoadTextureAt(&graphics, VITA_EXAMPLE_TEXTURE);

    GLFWwindow* glfwWin = graphics.GetGlfwWindow(&graphics);
    TeslaKeyboardInput keyboard = TKbd_Create();
    keyboard.InitBackend(&keyboard, glfwWin);
    // TODO: while(graphics.DoRun)
    while(graphics.private.doContinue)
    {
        keyboard.PollInput(&keyboard);
        CheckInput_keyboard(&keyboard);

        // Update. TODO: actual time keeping
        doUpdate(&graphics, 1 / 240.f);

        /*graphics.SetCamera(&graphics, 
            (vec3) {
                10.0f, 5.0f, -10.0f
            },
            (vec3) {
                0.f, 45.f, 0.f
            }
        );*/

        graphics.Clear(&graphics);
        graphics.Begin(&graphics);

        graphics.BindTexture(&graphics, thisTex);

        
        for(int x = 0; x < 20; x++)
        {
            for(int z = 0; z < 20; z++)
            {
                DrawCube(&graphics, (vec3){x * 1.0f, 0.f, z * 1.0f});                
            }
        }
        

        // DrawCube(&graphics, (vec3){0.f, 0.f, 0.f});
        // DrawCube(&graphics, (vec3){0.f, 2.f, 0.f});
        // DrawCube(&graphics, (vec3){0.f, -2.f, 0.f});
        /*
        // Begin Draw Sides
        graphics.DrawQuad(&graphics, 
            0.f, 0.f, -1.f, 
            (vec3){0.f, 0.f, 0.f}, 
            (vec3){SCALE,SCALE,SCALE}, 
            (vec4){1.0f, 1.0f, 1.0f, 1.0f}
        );
        graphics.DrawQuad(&graphics, 
            -0.5f, 0.f, -0.5f, 
            (vec3){0.f, 90.f, 0.f}, 
            (vec3){SCALE,SCALE,SCALE},
            (vec4){1.0f, 1.0f, 1.0f, 1.0f}
        );
        
        graphics.DrawQuad(&graphics, 
            0.f, 0.f, 0.f, 
            (vec3){0.f, 180.f, 0.f}, 
            (vec3){SCALE,SCALE,SCALE},
            (vec4){1.0f, 1.0f, 1.0f, 1.0f}
        );
        graphics.DrawQuad(&graphics, 
            0.5f, 0.f, -0.5f, 
            (vec3){0.f, 270.f, 0.f}, 
            (vec3){SCALE,SCALE,SCALE},
            (vec4){1.0f, 1.0f, 1.0f, 1.0f}
        );
        graphics.BindTexture(&graphics, 0);
        // End Draw Sides
        
        // Top
        graphics.DrawQuad(&graphics, 
            0.f, 1.5f, -0.5f, 
            (vec3){90.f, 0.f, 0.f}, 
            (vec3){SCALE,SCALE,SCALE},
            (vec4){0.0f,0.5f,0.f,1.0f}
        );

        // Bottom
        graphics.DrawQuad(&graphics, 
            0.f, 0.5f, -0.5f, 
            (vec3){90.f, 0.f, 0.f}, 
            (vec3){SCALE,SCALE,SCALE},
            (vec4){0.0f,1.f,0.f,1.0f}
        );
        */
        
        graphics.End(&graphics);
    }

    // Destroy created texture.
    graphics.DestroyTexture(&graphics, thisTex);
    graphics.DestroyBackend(&graphics); // Close the backend.
    graphics.DestroySelf(&graphics);    // Destroy any other private data here.
    NETLOG_VITA_FINISH();

    return 0;
}