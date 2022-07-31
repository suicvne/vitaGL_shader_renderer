#include <stdio.h>
#include <assert.h>

#include "vgl3d.h"
#include "input/tesla_input.h"
#include "mesh/mesh.h"


static float _CurTime = 0.0f;
static uint8_t _OpType = 0;
static int doSpin = 1;
static TeslaMesh ExampleModel;
static TeslaMesh CubeSkyboxThing;

static inline float lerp(float a, float b, float f)
{ return a + f * (b - a); }

void doUpdate(VGL3DContext* context, float dt)
{
    if(!doSpin) return;
    if(_OpType == 0)
        _CurTime += dt;
    else _CurTime -= dt;

    if(_CurTime > 1.0f || _CurTime < 0.0f)
        _OpType = !_OpType;

    // Manipulate the camera.
    vec3 closePos = {0.0f, 0.0f, -10.0f};

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
// #define NETDBG_IP_SERVER "192.168.1.204"
#define NETDBG_IP_SERVER "192.168.1.124"
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
#define VITA_EXAMPLE_TEXTURE     "app0:background2-1.png"
#define VITA_EXAMPLE_TEXTURE2    "app0:block-26.png"
#define VITA_IDENTITY_CUBE_GLB   "app0:unit_cube.glb"
#define VITA_EXAMPLE_MESH_GLB    "app0:monkey.glb"
#else
// Empty define.
#define NETLOG_VITA_FINISH()
#define VITA_EXAMPLE_TEXTURE     "../background2-1.png"
#define VITA_EXAMPLE_TEXTURE2    "../block-26.png"
#define VITA_IDENTITY_CUBE_GLB   "../unit_cube.glb"
#define VITA_EXAMPLE_MESH_GLB    "../monkey.glb"
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

void PrintCurProjectionType(VGL3DContext* graphics) {
    char* projTypeStr = NULL;

    #define PROJ_MAT_PROP_CASE(ENMV, STORAGE)\
    case (ENMV): STORAGE = #ENMV;break;\

    switch(graphics->private.projectionMatrixType) {
        PROJ_MAT_PROP_CASE(VGL3D_PROJECTION_ORTHOGRAPHIC, projTypeStr)
        PROJ_MAT_PROP_CASE(VGL3D_PROJECTION_PERSPECTIVE, projTypeStr)
        PROJ_MAT_PROP_CASE(VGL3D_PROJECTION_IDENTITY, projTypeStr)
        default: projTypeStr = "VGL3D_PROJECTION_UNKNOWN";
    }

    graphics->Log(graphics, "New Projection Type: %s (%d)\n", projTypeStr, graphics->private.projectionMatrixType);

    #undef PROJ_MAT_PROP_CASE
}

void SetCamForProjType(VGL3DContext* graphics) {

    const vec3 defaultPerspPos = {0.0f, 0.0f, -10.0f};
    const vec3 defaultPerspRot = { -30.0f, 0.0f, 0.0f };

    switch(graphics->private.projectionMatrixType) {
        case VGL3D_PROJECTION_PERSPECTIVE:
            graphics->SetCamera(graphics, (float*)defaultPerspPos, (float*)defaultPerspRot);
            break;
        case VGL3D_PROJECTION_ORTHOGRAPHIC:
        case VGL3D_PROJECTION_IDENTITY:
        default:
            graphics->SetCamera(graphics, (vec3){0.0f, 0.0f, 0.0f}, (vec3){0.0f, 0.0f, 0.0f});        
            break;
    }
}

#ifndef VITA
static vec3 manualCamPos = {0.f, 0.f, -10.f};
static vec3 manualCamRot = {0};

void CheckInput_keyboard(TeslaKeyboardInput* kbdInput, VGL3DContext* graphics) {

    // Spin Model 
    if(kbdInput->IsKeyHeld(kbdInput, GLFW_KEY_SPACE))
        doSpin = 1;
    else
        doSpin = 0;

    // Switch Projection Type.
    if(kbdInput->IsKeyDown(kbdInput, GLFW_KEY_P)) {

        // Increment
        VGL3D_ProjectionMatType nextType = graphics->private.projectionMatrixType+1;
        if(nextType == VGL3D_PROJECTION_LAST) nextType = VGL3D_PROJECTION_IDENTITY;

        graphics->SetProjectionType(graphics, nextType);
        PrintCurProjectionType(graphics);
    }

    // Move x/z
    #define SPEED 30.f
    #define SPEED_VAL (SPEED) * (1 / 120.f)
    #define UPDATE_CAM() graphics->SetCamera(graphics, manualCamPos, manualCamRot)

    if(kbdInput->IsKeyHeld(kbdInput, GLFW_KEY_W )) {
        manualCamPos[2] += SPEED_VAL;
        UPDATE_CAM();
    } else if(kbdInput->IsKeyHeld(kbdInput, GLFW_KEY_S)) {
        manualCamPos[2] -= SPEED_VAL;
        UPDATE_CAM();
    }

    if(kbdInput->IsKeyHeld(kbdInput, GLFW_KEY_A)) {
        manualCamPos[0] += SPEED_VAL;
        UPDATE_CAM();
    } else if(kbdInput->IsKeyHeld(kbdInput, GLFW_KEY_D)) {
        manualCamPos[0] -= SPEED_VAL;
        UPDATE_CAM();
    }

    if(kbdInput->IsKeyHeld(kbdInput, GLFW_KEY_LEFT)) {
        manualCamRot[1] += SPEED_VAL;
        UPDATE_CAM();
    } else if(kbdInput->IsKeyHeld(kbdInput, GLFW_KEY_RIGHT)) {
        manualCamRot[1] -= SPEED_VAL;
        UPDATE_CAM();
    }

    if(kbdInput->IsKeyHeld(kbdInput, GLFW_KEY_UP)) {
        manualCamRot[0] += SPEED_VAL;
        UPDATE_CAM();
    } else if(kbdInput->IsKeyHeld(kbdInput, GLFW_KEY_DOWN)) {
        manualCamRot[0] -= SPEED_VAL;
        UPDATE_CAM();
    }

    #undef SPEED_VAL
    #undef SPEED
}
#endif

void Test_CubeTest(VGL3DContext* graphics, VTEX thisTex) {
    graphics->BindTexture(graphics, thisTex);

    for (int x = 0; x < 20; x++)
    {
        for (int z = 0; z < 20; z++)
        {
            DrawCube(graphics, (vec3){x * 1.0f, 0.f, z * 1.0f});
        }
    }
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
    graphics.SetProjectionType(&graphics, VGL3D_PROJECTION_PERSPECTIVE);

    VTEX thisTex = graphics.LoadTextureAt(&graphics, VITA_EXAMPLE_TEXTURE);
    VTEX otherTex = graphics.LoadTextureAt(&graphics, VITA_EXAMPLE_TEXTURE2);

    #ifndef VITA
    GLFWwindow* glfwWin = graphics.GetGlfwWindow(&graphics);
    TeslaKeyboardInput keyboard = TKbd_Create();
    keyboard.InitBackend(&keyboard, (struct _GLFWwindow*)glfwWin);
    #endif

    // Test (create instances with fn ptrs assigned.)
    ExampleModel = TestMesh_Create();
    CubeSkyboxThing = TestMesh_Create();
    #ifdef VITA
    // Vita override.
    graphics.Log = NetLogForVita_private;
    ExampleModel.Log = NetLogForVita_private;
    CubeSkyboxThing.Log = NetLogForVita_private;
    #endif

    ExampleModel.Log(&ExampleModel, "Reading mesh from '%s'...", VITA_EXAMPLE_MESH_GLB);
    ExampleModel.ReadGLTFAtPath(&ExampleModel, VITA_EXAMPLE_MESH_GLB);
    ExampleModel.private.TextureGpuHandle = otherTex;

    CubeSkyboxThing.Log(&CubeSkyboxThing, "Reading cube from '%s'...", VITA_IDENTITY_CUBE_GLB);
    // CubeSkyboxThing.InitWithDefaultCube(&CubeSkyboxThing);
    CubeSkyboxThing.ReadGLTFAtPath(&CubeSkyboxThing, VITA_IDENTITY_CUBE_GLB);
    CubeSkyboxThing.private.TextureGpuHandle = thisTex;


    // bump
    doUpdate(&graphics, 1 / 240.f);
    while(graphics.private.doContinue)
    {
        /* =========== Update ============ */
    #ifndef VITA
        keyboard.PollInput(&keyboard);
        CheckInput_keyboard(&keyboard, &graphics);
    #endif

        // Update. TODO: actual time keeping
        doUpdate(&graphics, 1 / 240.f);
        /* ========= End Update =========== */

        /* =========== Graphics ============= */
        graphics.Clear(&graphics);
        graphics.Begin(&graphics);

        #define SCALE 1.f
        ExampleModel.DrawTranslate(
            &ExampleModel, 
            &graphics, 
            (vec3){0.f, 0.f, 0.f}, 
            (vec3){0.f, 0.f, 0.f}, 
            (vec3){SCALE, SCALE, SCALE}
        );

        #undef SCALE
        #define SCALE 100.f
        CubeSkyboxThing.DrawTranslate(
            &CubeSkyboxThing,
            &graphics,
            (vec3){0.f, 0.f, 0.f},
            (vec3){0.f, 0.f, 0.f},
            (vec3){SCALE, SCALE, SCALE}
        );
        #undef SCALE


        // Draw other shit here.

        graphics.End(&graphics);
        /* ========= End Graphics =========== */
    }

    // Free mesh resources that we allocated.
    // Since this is stack allocated cube we dont
    //  need to free the ExampleModel.
    ExampleModel.DestroySelf(&ExampleModel);

    #ifndef VITA
    // Free resources that keyboard may have needed to allocate.
    keyboard.DestroySelf(&keyboard);
    #endif

    // Destroy created texture.
    graphics.DestroyTexture(&graphics, thisTex);
    graphics.DestroyBackend(&graphics); // Close the backend.
    graphics.DestroySelf(&graphics);    // Destroy any other private data here.
    NETLOG_VITA_FINISH();

    return 0;
}