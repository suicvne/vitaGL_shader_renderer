#include <stdio.h>
#include <math.h>
#include <assert.h>

#define __BASIC_MAP_IMPL_
#include "basic_hash_map.h"


#include "vgl_renderer.h"
#include "load_texture.h"
#include "SHADERS.h"


#define DISPLAY_WIDTH_DEF 960.f
#define DISPLAY_HEIGHT_DEF 544.f

#ifdef VITA
#include <debugnet.h>
#ifndef NETDBG_IP_SERVER
#define NETDBG_IP_SERVER "192.168.0.45"
#endif
#ifndef NETDBG_PORT_SERVER
#define NETDBG_PORT_SERVER 18194
#endif

static char __string_buffer[512] = {0};
#endif

void debugPrintf(const char* path, ...)
{
#if VITA
    va_list argptr;
    va_start(argptr, path);
    vsprintf(__string_buffer, path, argptr);
    va_end(argptr);

    debugNetPrintf(DEBUG, __string_buffer);
#else
    va_list argptr;
    va_start(argptr, path);
    vfprintf(stdout, path, argptr);
    va_end(argptr);
#endif
}


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
    float speed;
    char enabled;
    
    // Contains pivot, scale, and rot data
    obj_extra_data *ex_data;
} _entity;

#ifndef ENTITY_COUNT
#define ENTITY_COUNT 3000
#endif

_entity _test_entities[ENTITY_COUNT];

int min(int a, int b)
{
    return (a < b) ? a : b;
}

int init_debug()
{
#ifdef VITA
    return debugNetInit(NETDBG_IP_SERVER, NETDBG_PORT_SERVER, DEBUG);
    
#else
    return 0;
#endif
}

void init_entities()
{
    for(int i = 0; i < ENTITY_COUNT; i++)
    {
        _test_entities[i].x = rand() % 1000;
        _test_entities[i].y = rand() % 1000;
        _test_entities[i].w = (rand() % 4) * 16.f;
        _test_entities[i].h = (rand() % 4) * 16.f;
        _test_entities[i].speed = min(((rand() % 2) / 2.f) * 2.f, 1.0f);

        // Setup ex_data
        _test_entities[i].ex_data = (obj_extra_data *)malloc(sizeof(obj_extra_data));
        memset(_test_entities[i].ex_data, 0, sizeof(obj_extra_data));
        _test_entities[i].ex_data->scale = 1.f;
    }
}

void free_entities()
{
    for(int i = 0; i < ENTITY_COUNT; i++)
    {
        free(_test_entities[i].ex_data);
    }
}

void update_entities(float _ticks)
{
    for(int i = 0; i < ENTITY_COUNT; i++)
    {
        _test_entities[i].x += (_test_entities[i].speed * ((rand() % 2) == 1 ? -1 : 1) * _ticks) / _ticks;
        _test_entities[i].y += (_test_entities[i].speed * ((rand() % 2) == 1 ? -1 : 1) * _ticks) / _ticks;
        
        _test_entities[i].ex_data->piv_x = _test_entities[i].x + (_test_entities[i].w * .5f);
        _test_entities[i].ex_data->piv_y = _test_entities[i].y + (_test_entities[i].h * .5f);
        _test_entities[i].ex_data->scale = sinf(_ticks * .1f) * 4.f;
        

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
    for(int i = 0; i < ENTITY_COUNT; i++)
    {
        Vita_DrawTextureAnimColorExData(
            _test_entities[i].x, _test_entities[i].y,
            _test_entities[i].w, _test_entities[i].h,
            Texture_1, _tex_1_w, _tex_1_h,
            0, 0, 16.f, 32.f,
            fabs(sin(_ticks)),
            fabs(sin(_ticks)),
            fabs(sin(_ticks)),
            1.f,
            _test_entities[i].ex_data
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

static const char *_frag2_shader = "../frag_mm_shadow.glsl";
#endif





int bm_test()
{
    const int map_size = 8;
    bm_map_t *new_map = create_basic_map(map_size);

    assert(new_map != NULL);
    assert(new_map->__last_id == 0);
    assert(new_map->tracked_elements == 0);
    assert(new_map->max_elements == map_size);
    assert(new_map->map != NULL);

    
    bm_key_t *test_key = put_basic_map(new_map, (void *)_frag_shader);
    assert(test_key != NULL);
    debugPrintf("[bm_test] test_key->id is %u\n", test_key->id);
    // assert(test_key->id > -1);
    assert(test_key->obj_ptr != NULL);
    assert(((const char*)test_key->obj_ptr) == _frag_shader);
    debugPrintf("[bm_test] String value of test_key ptr: `%s`\n", (const char*)(test_key->obj_ptr));
    assert(new_map->tracked_elements > 0);
    assert(new_map->tracked_elements != new_map->max_elements);

    bm_key_t *test_key_2 = get_at_basic_map(new_map, 0);
    assert(test_key_2 != NULL);
    assert(test_key_2->id == test_key->id);
    assert(test_key_2->obj_ptr == test_key->obj_ptr);

    bm_key_t *test_key_3 = get_by_id_basic_map(new_map, test_key->id);

    assert(test_key_3 != NULL);
    assert(test_key_3->id == test_key->id);
    assert(test_key_3->obj_ptr == test_key->obj_ptr);

    return 0;
}

static obj_extra_data test_data_1 = 
(obj_extra_data)
{
    0,
    256 + 128.f, 256 + 128.f,
    0.f, 0.f, 0.f,
    1.f
};

static obj_extra_data test_sprite_1 = 
(obj_extra_data)
{
    0,
    (DISPLAY_WIDTH_DEF / 2) + 100, (DISPLAY_HEIGHT_DEF / 2) + 100,
    0.f, 0.f, 0.f,
    1.f
};

static obj_extra_data test_sprite_2 = 
(obj_extra_data)
{
    0,
    (DISPLAY_WIDTH_DEF / 2) + 100, (DISPLAY_HEIGHT_DEF / 2) + 100,
    0.f, 0.f, 0.f,
    1.f
};

short should_render_overlay = 1;

// const float rgba[4][4] = 
// {
//     0.f, 0.f, 0.f, 0.5f,
//     1.f, 0.f, 0.f, 0.5f,
//     0.f, 1.f, 0.f, 0.5f,
//     0.f, 0.f, 1.f, 0.5f
// };

const float rgba[4][4] = 
{
    0.f, 0.f, 0.f, .75f,
    1.f, 0.f, 0.f, .75f,
    0.f, 1.f, 0.f, .75f,
    0.f, 0.f, 1.f, .75f
};

void render_overlay()
{
    // Vita_DrawRect4xColor(40, 40, DISPLAY_WIDTH_DEF - 40, DISPLAY_HEIGHT_DEF - 40, rgba[0], rgba[1], rgba[2], rgba[3]);
    Vita_DrawRect4xColor(10, 10, DISPLAY_WIDTH_DEF - 20, DISPLAY_HEIGHT_DEF - 20, rgba[0], rgba[1], rgba[2], rgba[3]);
}


int main()
{
    init_debug();
    // bm_test();
    //assert(sizeof(uint32_t) == sizeof(float));
    //printf("sizeof(void*) = %lu\n\tsizeof(float) = %lu\n", sizeof(void*), sizeof(float));

    initGL(debugPrintf);

    int retVal = 0;
    char *vert_shader = malloc(2), *frag_shader = malloc(2);
    size_t vert_shader_size, frag_shader_size;

    retVal = _Vita_ReadShaderFromFile(_vertex_shader, &vert_shader_size, &vert_shader);
    if(retVal != 0)
    {
        debugPrintf("ERROR: Could not load shader from %s\n", _vertex_shader);
        return -1;
    }
    debugPrintf("-----------------\n%s\n----------------\n", vert_shader);

    retVal = _Vita_ReadShaderFromFile(_frag_shader, &frag_shader_size, &frag_shader);
    if(retVal != 0)
    {
        debugPrintf("ERROR: Could not load frag shader from %s\n", _frag_shader);
        return -1;
    }
    debugPrintf("-----------------\n%s\n----------------\n", frag_shader);

    int shadingErrorCode = 0;

    if((shadingErrorCode = initGLShading2(vert_shader, frag_shader)) != 0)
    {
        debugPrintf("ERROR INIT GL SHADING! %d\n", shadingErrorCode);
        return -1;
    }

    free(vert_shader);
    free(frag_shader);
    
    initGLAdv();


#ifndef VITA

    char *frag2 = malloc(2);
    size_t frag2_size;
    retVal = _Vita_ReadShaderFromFile(_frag2_shader, &frag2_size, &frag2);
    if(retVal != 0)
    {
        debugPrintf("ERROR: Could not load frag shader 2.");
    }
    else
    {
        debugPrintf("-----------------\n%s\n----------------\n", frag2);
        if(Vita_AddShaderPass(NULL, frag2, -1) != 0)
        {
            debugPrintf("No secondary shader.\n");
            return -1;
        }
    }
#endif








    void* tex_buffer = malloc(8);
    int channels = 0;

    Vita_LoadTextureBuffer(_texture_1_path, &tex_buffer, &_tex_1_w, &_tex_1_h, &channels, (void*)debugPrintf);
    debugPrintf("[main] tex_buffer: %p\n", tex_buffer);

    Texture_1 = Vita_LoadTextureGL(tex_buffer, _tex_1_w, _tex_1_h, (void*)debugPrintf);
    if(Texture_1 == 0)
    {
        debugPrintf("Texture_1 failed to load: Returned %d for ID.\n", Texture_1);
        deInitGL();
        return -1;
    }
    else free(tex_buffer);

    init_entities();
    

    int run = 1;

    while(run == 1)
    {
        // clear current vbo
        Vita_Clear();

        Vita_DrawRectColor(fabs(cos(_ticks * .5f) * (300)) + 4, 4, 32, 32, .2f, .2f, .2f, .58f);
        Vita_Draw(fabs(cos(_ticks * .5f) * (300 + 0)), 0, 32, 32);

        Vita_DrawRectColor(fabs(cos(_ticks * .5f) * (300 + 20)) + 4, 36, 64, 64, .2f, .2f, .2f, .58f);
        Vita_Draw(fabs(cos(_ticks * .5f) * (300 + 20)), 32, 64, 64);

        Vita_DrawRectColor(fabs(cos(_ticks * .5f) * (300 + 40)) + 4, (sin(_ticks) * (32 + 64)) + 4, 128, 128, .2f, .2f, .2f, .58f);
        Vita_Draw(fabs(cos(_ticks * .5f) * (300 + 40)), (sin(_ticks) * (32 + 64)) , 128, 128);

        test_data_1.rot_z = sin((_ticks * .2f)) * 360.f;

        Vita_DrawRectColorExData(
            256.f, 256.f,
            256.f, 256.f,
            1.f, 0.f, 0.f, 1.0f,
            &test_data_1 // sin((_ticks * .2f)) * 360.f (rotation)
        );

        float _scale_w = sin(_ticks) * (64.f * 2);
        float _scale_h = cos(_ticks) * (128.f * 2);
        float _half_w = _scale_w * .5f;
        float _half_h = _scale_h * .5f;

        test_sprite_1.scale = 1;
        test_sprite_2.scale = 1;

        test_sprite_1.rot_z = sin((_ticks * .2f)) * 360.f;

        Vita_DrawTextureAnimColorExData(
            (DISPLAY_WIDTH_DEF / 2) - _half_w, // screen x
            (DISPLAY_HEIGHT_DEF / 2) - _half_h, // screen y
            128.f, 128.f, // W & H on screen.
            Texture_1, _tex_1_w, _tex_1_h, // Tex ID, tex w, tex h
            0, 0, 16, 32, // Src X, Src Y, Src W, Src H
            1.f, 1.f, 1.f, 1.f, // Color,
            &test_sprite_1
        ); 

        Vita_DrawTextureAnimColorExData(
            (DISPLAY_WIDTH_DEF / 2) - _half_w - 100, // screen x
            (DISPLAY_HEIGHT_DEF / 2) - _half_h - 100, // screen y
            _scale_w, _scale_h, // W & H on screen.
            Texture_1, _tex_1_w, _tex_1_h, // Tex ID, tex w, tex h
            0, 0, 16, 32, // Src X, Src Y, Src W, Src H
            1.f, 1.f, 1.f, 1.f, // Color
            &test_sprite_2 // sin((_ticks * .2f)) * 360.f // Rot.
        ); 

        render_entities();

        render_overlay();
        

        // Vita_DrawRectColorRot(0, 0, 9.5f, 9.5f, 0.f, 1.f, 0.f, 0.2f, 1.0f);


        // Draw from vbo, swap to next vbo
        Vita_Repaint();
#ifdef VITA
        _ticks += .077f;
#else
        _ticks += 0.0077f;
#endif

        update_entities(_ticks);
    }


    return 0;
}
