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
        .Draw = TestMesh_Draw,
        .DrawTranslate = TestMesh_DrawTranslate,

        .private.MeshGpuHandle = 0,
        .private.TextureGpuHandle = 0,
    };
}

static inline void TestMesh_private_CheckUploadOrRefresh(SELF, VGL3DContext* graphics) {
    // Have we uploaded the data?
    if(pSelf->private.MeshGpuHandle == 0 || pSelf->pHasChanged) {
        pSelf->pHasChanged = 0;
        graphics->Log(graphics, "Mesh is uploading data....");
        // No, we haven't. Let's do that!
        pSelf->private.MeshGpuHandle = 
            graphics->CreateVBOWithVertexData(graphics, (const float*)pSelf->pVertices, pSelf->pNumVertices);
    }
}

void TestMesh_DrawTranslate(SELF, VGL3DContext* graphics, vec3 pos, vec3 rot, vec3 scale) {

    TestMesh_private_CheckUploadOrRefresh(pSelf, graphics);

    // Call the bind texture & draw from VBO commands.
    pSelf->Draw(pSelf, graphics);

    // Bind Texture.
    graphics->BindTexture(graphics, pSelf->private.TextureGpuHandle);

    // Draw from VBO.
    graphics->DrawFromVBOTranslation(
        graphics, 
        pSelf->private.MeshGpuHandle, 
        pSelf->pNumVertices, 
        pos, 
        rot, 
        scale
    );

}

void TestMesh_Draw(SELF, VGL3DContext* graphics) {

    TestMesh_private_CheckUploadOrRefresh(pSelf, graphics);

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
    //      pos (xyz)                   (texCoordU/V)
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