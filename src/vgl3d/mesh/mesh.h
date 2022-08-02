#ifndef __MESH_H__
#define __MESH_H__

#include <stdlib.h>
#include "../vgl3d.h"
#include "../common.h"

#define SELF struct _Mesh* pSelf

typedef struct _Vertex {
    union {
        vec3  position;
        float x, y, z;
    };

    union {
        vec2  uv;
        float texCoordU, texCoordV;
    };
} Vertex;

struct _Mesh    TestMesh_Create();
struct _Mesh*   TestMesh_CreateHeap();
int             TestMesh_InitWithVertices(SELF, const Vertex* allocatedVertexData, size_t vertexCount);
int             TestMesh_InitWithDefaultCube(SELF);
void            TestMesh_DestroySelf(SELF);
void            TestMesh_Draw(SELF, VGL3DContext* graphics);
void            TestMesh_DrawTranslate(SELF, VGL3DContext* graphics, vec3 pos, vec3 rot, vec3 scale);
int             TestMesh_ReadGLTFAtPath(SELF, const char* path);

typedef struct _Mesh {

    void        (*Log)(SELF, const char* fmt, ...);
    int         (*InitWithVertices)(SELF, const Vertex* allocatedVertexData, size_t vertexDataSize);
    int         (*InitWithDefaultCube)(SELF);
    void        (*DestroySelf)(SELF);
    void        (*Draw)(SELF, VGL3DContext* graphics);
    void        (*DrawTranslate)(SELF, VGL3DContext* graphics, vec3 pos, vec3 rot, vec3 scale);
    int         (*ReadGLTFAtPath)(SELF, const char* path);

    Vertex*         pVertices;
    uint32_t*       pIndices;
    uint32_t        pNumIndices;
    uint32_t        pNumVertices;
    uint8_t         pHasChanged:1;
    uint8_t         pFreeAfterUpload:1;

    C_PRIVATE_BEGIN(TestMesh)
    uint32_t    MeshGpuHandle;
    uint32_t    TextureGpuHandle;
    C_PRIVATE_END;
    
} TeslaMesh;

CREATE_LOG_FN(SELF, TestMesh, "MeshReader")

#undef SELF

#endif //__MESH_H__