#include <stdio.h>
#include <assert.h>

#include "vgl3d.h"
#include "input/tesla_input.h"
#include "mesh/mesh.h"

static inline float lerp(float a, float b, float f)
{ return a + f * (b - a); }

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
#define VITA_EXAMPLE_TEXTURE    "app0:background2-1.png"
#define VITA_EXAMPLE_MESH_GLB   "app0:cone_test.glb"
#else
// Empty define.
#define NETLOG_VITA_FINISH()
#define VITA_EXAMPLE_TEXTURE    "../background2-1.png"
#define VITA_EXAMPLE_MESH_GLB   "../cone_test_gltf.gltf"
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
    #define SPEED 8.f
    if(kbdInput->IsKeyHeld(kbdInput, GLFW_KEY_UP )) {
        manualCamPos[2] -= SPEED * (1 / 120.f);
        graphics->SetCamera(graphics, manualCamPos, manualCamRot);
    } else if(kbdInput->IsKeyHeld(kbdInput, GLFW_KEY_DOWN)) {
        manualCamPos[2] += SPEED * (1 / 120.f);
        graphics->SetCamera(graphics, manualCamPos, manualCamRot);
    }
    #undef SPEED
}

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

void Test_PerspectiveSwap_Update(VGL3DContext* graphics, VTEX thisTex) {
    
}

static TeslaMesh DefaultCube;
void Test_PerspectiveSwap(VGL3DContext* graphics, VTEX thisTex) {
    
    /*
    // TODO: Do something about this >.> 
    // Add some getter functions or something.
    const float w = 960.f;
    const float h = 544.f;

    // Render orthographic background.
    graphics->SetProjectionType(graphics, VGL3D_PROJECTION_ORTHOGRAPHIC);
    SetCamForProjType(graphics);
    graphics->BindTexture(graphics, thisTex);
    graphics->DrawQuad(
        graphics, 
        0.f,0.f,0.f, 
        (vec3){0.f,0.f,0.f}, 
        (vec3){w, h, 1.f},
        (vec4){1.f,1.f,1.f,1.f}
    );
    */
    
    // Render Other tests on top.
    
    graphics->SetProjectionType(graphics, VGL3D_PROJECTION_PERSPECTIVE);
    // SetCamForProjType(graphics);
    
    // Test_CubeTest(graphics, thisTex);
    DefaultCube.Draw(&DefaultCube, graphics);

    // doUpdate(graphics, 1 / 120.f);
}


void Test_MeshTest(VGL3DContext* graphics, const char* path) {
    // TODO: Draw the mesh or smth.
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

    GLFWwindow* glfwWin = graphics.GetGlfwWindow(&graphics);
    TeslaKeyboardInput keyboard = TKbd_Create();
    keyboard.InitBackend(&keyboard, (struct _GLFWwindow*)glfwWin);

    // Test
    DefaultCube = TestMesh_Create();
    DefaultCube.Log(&DefaultCube, "Reading mesh from '%s'...", VITA_EXAMPLE_MESH_GLB);
    DefaultCube.ReadGLTFAtPath(&DefaultCube, VITA_EXAMPLE_MESH_GLB);

    // bump
    doUpdate(&graphics, 1 / 240.f);
    while(graphics.private.doContinue)
    {
        /* =========== Update ============ */
        keyboard.PollInput(&keyboard);
        CheckInput_keyboard(&keyboard, &graphics);

        // Update. TODO: actual time keeping
        doUpdate(&graphics, 1 / 240.f);
        /* ========= End Update =========== */

        /* =========== Graphics ============= */
        graphics.Clear(&graphics);
        graphics.Begin(&graphics);

        /*
        DefaultCube.DrawTranslate(&DefaultCube, 
            &graphics, 
            (vec3){0.f, 0.f, 0.f}, 
            (vec3){0.f, 0.f, 0.f}, 
            (vec3){0.12f, 0.12f, 0.12f});
            */

        DrawCube(&graphics, (vec3){2.0f, 0.f, 5 * 1.0f});
        
        // DefaultCube.Draw(&DefaultCube, &graphics);
        // Test_CubeTest(&graphics, thisTex);
        // Test_PerspectiveSwap(&graphics, thisTex);


        // Draw other shit here.

        graphics.End(&graphics);
        /* ========= End Graphics =========== */
    }

    // Free mesh resources that we allocated.
    // Since this is stack allocated cube we dont
    //  need to free the DefaultCube.
    DefaultCube.DestroySelf(&DefaultCube);

    // Free resources that keyboard may have needed to allocate.
    keyboard.DestroySelf(&keyboard);

    // Destroy created texture.
    graphics.DestroyTexture(&graphics, thisTex);
    graphics.DestroyBackend(&graphics); // Close the backend.
    graphics.DestroySelf(&graphics);    // Destroy any other private data here.
    NETLOG_VITA_FINISH();

    return 0;
}