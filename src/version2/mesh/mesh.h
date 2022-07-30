#ifndef __MESH_H__
#define __MESH_H__

#include <stdlib.h>
#include "../vgl3d.h"

#define SELF struct _Mesh* pSelf

typedef struct _Vertex {
    float x, y, z;
    float u, v;
} Vertex;

struct _Mesh TestMesh_Create();
int TestMesh_InitWithVertices(SELF, const Vertex* allocatedVertexData, size_t vertexCount);
int TestMesh_InitWithDefaultCube(SELF);
void TestMesh_DestroySelf(SELF);
void TestMesh_DrawSlow(SELF, VGL3DContext* graphics);

typedef struct _Mesh {

    int (*InitWithVertices)(SELF, const Vertex* allocatedVertexData, size_t vertexDataSize);
    int (*InitWithDefaultCube)(SELF);
    void (*DestroySelf)(SELF);
    void (*DrawSlow)(SELF, VGL3DContext* graphics);

    Vertex* pVertices;
    size_t  pNumVertices;
    uint8_t   pHasChanged;

    C_PRIVATE_BEGIN(TestMesh)
    uint32_t MeshGpuHandle;
    uint32_t TextureGpuHandle;
    C_PRIVATE_END;
} Test_Mesh;

#undef SELF

#endif //__MESH_H__