#ifndef __VGL_RENDERER_TYPES_H__
#define __VGL_RENDERER_TYPES_H__

typedef struct _vert 
{
    float x, y, z;
    float s, v; // Tex Coord X, Tex Coord Y
    float _r, _g, _b, _a;

    void *obj_ptr;
} __attribute__ ((packed)) vert;

typedef struct _obj_extra_data 
{
    unsigned int textureID;
    float piv_x;
    float piv_y;
    float rot_x;
    float rot_y;
    float rot_z;
    float scale;
} __attribute__ ((packed)) obj_extra_data;

typedef struct _shading_pass
{
    unsigned int ProgramObjectID;
    unsigned int VertexShaderID;
    unsigned int FragmentShaderID;
    float offset_x;
    float offset_y;
} __attribute__ ((packed)) ShadingPass;

typedef struct _DrawCall
{
    union
    {
        struct _vert verts_quad[4];
        struct _vert verts_tri[3];
    } draw;
} __attribute__ ((packed)) DrawCall;

typedef struct _DrawCall3
{
    union
    {
        struct _vert verts_quad[4];
        struct _vert verts_tri[3];
    } draw;
    // uint8_t draw_type;
} __attribute__ ((packed)) DrawCallTEST;


typedef struct _DrawCall2
{
    struct _vert verts_quad[4];
} __attribute__ ((packed)) DrawCallOld;

#endif // __VGL_RENDERER_TYPES_H__