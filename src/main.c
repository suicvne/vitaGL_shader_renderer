#include <stdio.h>
#include <math.h>

#include "vgl_renderer.h"
#include "load_texture.h"
#include "SHADERS.h"

#define DISPLAY_WIDTH_DEF 960.f
#define DISPLAY_HEIGHT_DEF 544.f


const GLint _scr_offset_x = (DISPLAY_WIDTH_DEF);
const GLint _scr_offset_y = (DISPLAY_HEIGHT_DEF);

static GLuint Texture_1 = 0;
static GLint _tex_1_w = 0;
static GLint _tex_1_h = 0;

float _ticks = 0;

typedef struct _e
{
    float x;
    float y;
    float w; 
    float h;
    float scale;
    float speed;
    char enabled;
    float rot_x;
    float rot_y;
} _entity;

_entity _test_entities[32];

int min(int a, int b)
{
    return (a < b) ? a : b;
}

void init_entities()
{
    for(int i = 0; i < 32; i++)
    {
        _test_entities[i].x = rand() % 1000;
        _test_entities[i].y = rand() % 1000;
        _test_entities[i].scale = 1.f;
        _test_entities[i].w = (rand() % 4) * 16.f;
        _test_entities[i].h = (rand() % 4) * 16.f;
        _test_entities[i].speed = min(((rand() % 2) / 2.f) * 2.f, 1.0f);
        _test_entities[i].rot_x = 0;
        _test_entities[i].rot_y = 0;
    }
}

void update_entities(float _ticks)
{
    for(int i = 0; i < 32; i++)
    {
        _test_entities[i].x += (_test_entities[i].speed * ((rand() % 2) == 1 ? -1 : 1) * _ticks) / _ticks;
        _test_entities[i].y += (_test_entities[i].speed * ((rand() % 2) == 1 ? -1 : 1) * _ticks) / _ticks;
        
        _test_entities[i].scale = sinf(_ticks * .1f) * 4.f;
        

        if(_test_entities[i].x > DISPLAY_WIDTH_DEF)
            _test_entities[i].x = DISPLAY_WIDTH_DEF;
        else if(_test_entities[i].x < 0)
            _test_entities[i].x = 0;

        if(_test_entities[i].y > DISPLAY_HEIGHT_DEF)
            _test_entities[i].y = DISPLAY_HEIGHT_DEF;
        else if(_test_entities[i].y < 0)
            _test_entities[i].y = 0;
    }
}

void render_entities()
{
    for(int i = 0; i < 32; i++)
    {
        Vita_DrawTextureAnimColorRotScale(
            _test_entities[i].x, _test_entities[i].y,
            _test_entities[i].w, _test_entities[i].h,
            Texture_1, _tex_1_w, _tex_1_h,
            0, 0, 16.f, 32.f,
            fabs(sin(_ticks)),
            fabs(sin(_ticks)),
            fabs(sin(_ticks)),
            1.f,
            0.f,
            _test_entities[i].scale
        );
    }
}

#ifdef VITA
static const char *_texture_1_path = "app0:bobomb_red.png";
static const char *_vertex_shader = "app0:vert.cgv";
static const char *_frag_shader = "app0:frag.cgf";
#else
static const char *_texture_1_path = "../bobomb_red.png";
static const char *_vertex_shader = "../vert.glsl";
static const char *_frag_shader = "../frag.glsl";
#endif
int main()
{
    initGL();

    int retVal = 0;
    char *vert_shader = malloc(2), *frag_shader = malloc(2);
    size_t vert_shader_size, frag_shader_size;

    retVal = _Vita_ReadShaderFromFile(_vertex_shader, &vert_shader_size, &vert_shader);
    if(retVal != 0)
    {
        printf("ERROR: Could not load shader from %s\n", _vertex_shader);
        return -1;
    }

    retVal = _Vita_ReadShaderFromFile(_frag_shader, &frag_shader_size, &frag_shader);
    if(retVal != 0)
    {
        printf("ERROR: Could not load frag shader from %s\n", _frag_shader);
        return -1;
    }

    int shadingErrorCode = 0;

    if((shadingErrorCode = initGLShading2(vert_shader, frag_shader)) != 0)
    {
        printf("ERROR INIT GL SHADING! %d\n", shadingErrorCode);
        return -1;
    }

    free(vert_shader);
    free(frag_shader);
    
    initGLAdv();

    void* tex_buffer = malloc(8);
    int channels = 0;

    Vita_LoadTextureBuffer(_texture_1_path, &tex_buffer, &_tex_1_w, &_tex_1_h, &channels, (void*)printf);
    printf("[main] tex_buffer: %p\n", tex_buffer);

    Texture_1 = Vita_LoadTextureGL(tex_buffer, _tex_1_w, _tex_1_h, (void*)printf);
    if(Texture_1 == 0)
    {
        printf("Texture_1 failed to load: Returned %d for ID.\n", Texture_1);
        deInitGL();
        return -1;
    }
    else free(tex_buffer);

    init_entities();
    

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

        Vita_DrawRectColorRot(
            256.f, 256.f,
            256.f, 256.f,
            sin((_ticks * .2f)) * 360.f,
            1.f, 0.f, 0.f, 1.0f
        );

        float _scale_w = sin(_ticks) * (64.f * 2);
        float _scale_h = cos(_ticks) * (128.f * 2);
        float _half_w = _scale_w * .5f;
        float _half_h = _scale_h * .5f;

        Vita_DrawTextureAnimColor(
            (DISPLAY_WIDTH_DEF / 2) - _half_w, // screen x
            (DISPLAY_HEIGHT_DEF / 2) - _half_h, // screen y
            _scale_w, _scale_h, // W & H on screen.
            Texture_1, _tex_1_w, _tex_1_h, // Tex ID, tex w, tex h
            0, 0, 16, 32, // Src X, Src Y, Src W, Src H
            1.f, 1.f, 1.f, 1.f // Color
        ); 

        Vita_DrawTextureAnimColorRot(
            (DISPLAY_WIDTH_DEF / 2) - _half_w - 100, // screen x
            (DISPLAY_HEIGHT_DEF / 2) - _half_h - 100, // screen y
            _scale_w, _scale_h, // W & H on screen.
            Texture_1, _tex_1_w, _tex_1_h, // Tex ID, tex w, tex h
            0, 0, 16, 32, // Src X, Src Y, Src W, Src H
            1.f, 1.f, 1.f, 1.f, // Color
            sin((_ticks * .2f)) * 360.f // Rot.
        ); 

        // render_entities();

        Vita_DrawRectColorRot(0, 0, 9.5f, 9.5f, 0.f, 1.f, 0.f, 0.2f, 1.0f);


        // Draw from vbo, swap to next vbo
        repaint();
        _ticks += .077f;

        // update_entities(_ticks);
    }


    return 0;
}
