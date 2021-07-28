#include <stdio.h>
#include <math.h>

#include "vgl_renderer.h"
#include "load_texture.h"

#define DISPLAY_WIDTH_DEF 960.f
#define DISPLAY_HEIGHT_DEF 544.f


const GLint _scr_offset_x = (DISPLAY_WIDTH_DEF);
const GLint _scr_offset_y = (DISPLAY_HEIGHT_DEF);

static GLuint Texture_1 = 0;

float _ticks = 0;

int main()
{
    initGL();

    int shadingErrorCode = 0;

    if((shadingErrorCode = initGLShading()) != 0)
    {
        printf("ERROR INIT GL SHADING! %d\n", shadingErrorCode);
        return -1;
    }
    
    initGLAdv();

    void* tex_buffer = malloc(8);
    int w = 0, h = 0, channels = 0;

    Vita_LoadTextureBuffer("bobomb_red.png", &tex_buffer, &w, &h, &channels, (void*)printf);
    printf("[main] tex_buffer: %x, %p", tex_buffer, tex_buffer);

    Texture_1 = Vita_LoadTextureGL(tex_buffer, w, h, (void*)printf);
    if(Texture_1 == 0)
    {
        printf("Texture_1 failed to load: Returned %d for ID.\n", Texture_1);
        deInitGL();
        return -1;
    }
    

    int run = 1;

    while(run == 1)
    {
        // clear current vbo
        clear();

        // for(int i = 0; i < 1000; i++)
        // {
        //     Vita_Draw(
        //         (sin(_ticks) * 34) + 272.f + (sin(_ticks / 2) * (18.f * i)), 
        //         (cos(_ticks) * 34) + 256.f + (sin(16.f) * i), 
        //         30, 
        //         30
        //     );
        // }

        Vita_Draw(fabs(cos(_ticks * .5f) * (300 + 0)), 0, 32, 32);
        Vita_Draw(fabs(cos(_ticks * .5f) * (300 + 20)), 32, 64, 64);
        Vita_Draw(fabs(cos(_ticks * .5f) * (300 + 40)), sin(_ticks) * (32 + 64) , 128, 128);

        // Draw from vbo, swap to next vbo
        repaint();

        _ticks += .077f;
    }


    return 0;
}
