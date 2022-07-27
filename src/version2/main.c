#include <stdio.h>
#include "vgl3d.h"

static inline float lerp(float a, float b, float f)
{
    return a + f * (b - a);
}

static float _CurTime = 0.0f;
static uint8_t _OpType = 0;

void doUpdate(VGL3DContext* context, float dt)
{
    if(_OpType == 0)
        _CurTime += dt;
    else _CurTime -= dt;

    if(_CurTime > 1.0f || _CurTime < 0.0f)
        _OpType = !_OpType;

    // Manipulate the camera.
    vec3 closePos = (vec3){0.0f, 0.0f, -2.0f};
    vec3 farPos = (vec3){-2.0f, 0.5f, -10.0f};
    vec3 lerpedPos = {0};

    vec3 closeRot = (vec3) { 0.0f, 0.0f, 0.0f };
    vec3 farRot = (vec3){ -20.0f, -360.f, 20.0f };
    vec3 lerpedRot = { 0 };

    glm_vec3_lerp(closePos, farPos, _CurTime, lerpedPos);
    glm_vec3_lerp(closeRot, farRot, _CurTime, lerpedRot);

    // Update camera position.
    context->SetCamera(context, lerpedPos, lerpedRot);
}

int main()
{
    printf("Hello world!\n");

    VGL3DContext graphics = VGL3D_Create();
    graphics.InitBackend(&graphics);
    graphics.Log(&graphics, "LOGS TEST! %d", 69);

    VTEX thisTex = graphics.LoadTextureAt(&graphics, "../background2-1.png");

    // TODO: while(graphics.DoRun)
    while(graphics.private.doContinue)
    {
        // Update
        doUpdate(&graphics, 1 / 240.f);

        graphics.Clear(&graphics);
        graphics.Begin(&graphics);

        #define SCALE 1.0f

        graphics.BindTexture(&graphics, thisTex);
        /** Begin Draw Sides **/
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
        /** End Draw Sides **/
        
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
        
        graphics.End(&graphics);
    }

    return 0;
}