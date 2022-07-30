#include "mesh.h"
#include <memory.h>
#include <assert.h>

#define SELF TeslaMesh* pSelf

TeslaMesh TestMesh_Create() {
    return (TeslaMesh) {
        .pVertices = NULL,
        .pNumVertices = 0,
        .pHasChanged = 0,

        .InitWithDefaultCube = TestMesh_InitWithDefaultCube,
        .DestroySelf = TestMesh_DestroySelf,
        .InitWithVertices = TestMesh_InitWithVertices,
        .Draw = TestMesh_DrawSlow,

        .private.MeshGpuHandle = 0,
        .private.TextureGpuHandle = 0,
    };
}

void TestMesh_DrawSlow(SELF, VGL3DContext* graphics) {

    // Have we uploaded the data?
    if(pSelf->private.MeshGpuHandle == 0) {
        graphics->Log(graphics, "Mesh is uploading data....");
        // No, we haven't. Let's do that!
        pSelf->private.MeshGpuHandle = 
            graphics->CreateVBOWithVertexData(graphics, (const float*)pSelf->pVertices, pSelf->pNumVertices);
    }

    // Bind the texture.
    graphics->BindTexture(graphics, pSelf->private.TextureGpuHandle);

    // Draw.
    graphics->DrawFromVBO(graphics, pSelf->private.MeshGpuHandle, pSelf->pNumVertices);
}

void TestMesh_DestroySelf(SELF) {
    if(pSelf != NULL && pSelf->pVertices != NULL) {
        free(pSelf->pVertices);
        pSelf->pNumVertices = 0;
    }
}

int TestMesh_InitWithVertices(SELF, const Vertex* allocatedVertexData, size_t vertexCount) {
    if(allocatedVertexData == NULL) return -1;
    
    if(pSelf->pVertices == NULL) 
    {
        pSelf->pVertices = malloc(sizeof(Vertex) * vertexCount);
        pSelf->pNumVertices = vertexCount;
    } 
    else if(pSelf->pVertices != NULL && 
        pSelf->pNumVertices != vertexCount) 
    {
        // Realloc buffer.
        pSelf->pVertices = realloc(pSelf->pVertices, sizeof(Vertex) * vertexCount);
        pSelf->pNumVertices = vertexCount;
    }

    // Copy the data
    memcpy(pSelf->pVertices, allocatedVertexData, sizeof(Vertex) * vertexCount);

    return 0;
}

#define MESH_CUBE_VB_LENGTH 36
static const Vertex pVertexBufferData[] = {
    (Vertex){-1.0f,-1.0f,-1.0f,       0.0f, 1.0f},      // triangle 1 : begin
    (Vertex){-1.0f,-1.0f, 1.0f,       0.0f, 0.0f},
    (Vertex){-1.0f, 1.0f, 1.0f,       1.0f, 0.0f},      // triangle 1 : end
    (Vertex){1.0f, 1.0f,-1.0f,        0.0f, 1.0f},      // triangle 2 : begin
    (Vertex){-1.0f,-1.0f,-1.0f,       0.0f, 0.0f},
    (Vertex){-1.0f, 1.0f,-1.0f,       1.0f, 0.0f},      // triangle 2 : end
    (Vertex){1.0f,-1.0f, 1.0f,        0.0f, 1.0f},
    (Vertex){-1.0f,-1.0f,-1.0f,       0.0f, 0.0f},
    (Vertex){1.0f,-1.0f,-1.0f,        1.0f, 0.0f},
    (Vertex){1.0f, 1.0f,-1.0f,        0.0f, 1.0f},
    (Vertex){1.0f,-1.0f,-1.0f,        0.0f, 0.0f},
    (Vertex){-1.0f,-1.0f,-1.0f,       1.0f, 0.0f},
    (Vertex){-1.0f,-1.0f,-1.0f,       0.0f, 1.0f},
    (Vertex){-1.0f, 1.0f, 1.0f,       0.0f, 0.0f},
    (Vertex){-1.0f, 1.0f,-1.0f,       1.0f, 0.0f},
    (Vertex){1.0f,-1.0f, 1.0f,        0.0f, 1.0f},
    (Vertex){-1.0f,-1.0f, 1.0f,       0.0f, 0.0f},
    (Vertex){-1.0f,-1.0f,-1.0f,       1.0f, 0.0f},
    (Vertex){-1.0f, 1.0f, 1.0f,       0.0f, 1.0f},
    (Vertex){-1.0f,-1.0f, 1.0f,       0.0f, 0.0f},
    (Vertex){1.0f,-1.0f, 1.0f,        1.0f, 0.0f},
    (Vertex){1.0f, 1.0f, 1.0f,        0.0f, 1.0f},
    (Vertex){1.0f,-1.0f,-1.0f,        0.0f, 0.0f},
    (Vertex){1.0f, 1.0f,-1.0f,        1.0f, 0.0f},
    (Vertex){1.0f,-1.0f,-1.0f,        0.0f, 1.0f},
    (Vertex){1.0f, 1.0f, 1.0f,        0.0f, 0.0f},
    (Vertex){1.0f,-1.0f, 1.0f,        1.0f, 0.0f},
    (Vertex){1.0f, 1.0f, 1.0f,        0.0f, 1.0f},
    (Vertex){1.0f, 1.0f,-1.0f,        0.0f, 0.0f},
    (Vertex){-1.0f, 1.0f,-1.0f,       1.0f, 0.0f},
    (Vertex){1.0f, 1.0f, 1.0f,        0.0f, 1.0f},
    (Vertex){-1.0f, 1.0f,-1.0f,       0.0f, 0.0f},
    (Vertex){-1.0f, 1.0f, 1.0f,       1.0f, 0.0f},
    (Vertex){1.0f, 1.0f, 1.0f,        0.0f, 1.0f},
    (Vertex){-1.0f, 1.0f, 1.0f,       0.0f, 0.0f},
    (Vertex){1.0f,-1.0f, 1.0f,        1.0f, 0.0f},
};

// TODO: Rename this to default *****quad******
int TestMesh_InitWithDefaultCube(SELF) {

    // Initailize with the data, 36 vertices.
    pSelf->InitWithVertices(pSelf, pVertexBufferData, MESH_CUBE_VB_LENGTH);

    return 0;
}

#undef SELF