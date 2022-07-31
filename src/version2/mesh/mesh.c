#include "mesh.h"
#include <memory.h>
#include <assert.h>

#define CGLTF_IMPLEMENTATION
#include "formats/cgltf.h"

#define SELF TeslaMesh* pSelf

TeslaMesh TestMesh_Create() {
    return (TeslaMesh) {
        .pVertices = NULL,
        .pIndices  = NULL,
        .pNumIndices = 0,
        .pNumVertices = 0,
        .pHasChanged = 0,

        .Log =                  TestMesh_Log,
        .InitWithDefaultCube =  TestMesh_InitWithDefaultCube,
        .DestroySelf =          TestMesh_DestroySelf,
        .InitWithVertices =     TestMesh_InitWithVertices,
        .Draw =                 TestMesh_Draw,
        .DrawTranslate =        TestMesh_DrawTranslate,

        .ReadGLTFAtPath =       TestMesh_ReadGLTFAtPath,

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

    // Bind Texture.
    graphics->BindTexture(graphics, pSelf->private.TextureGpuHandle);

    // Do we have indices?
    if(pSelf->pNumIndices > 0 && pSelf->pIndices != NULL) {
        graphics->DrawFromVBOTranslationIndices(
            graphics,
            pSelf->private.MeshGpuHandle,
            pSelf->pNumVertices,
            pos, rot, scale,
            pSelf->pIndices, pSelf->pNumIndices
        );
    }
    else {
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

}

void TestMesh_Draw(SELF, VGL3DContext* graphics) {

    TestMesh_private_CheckUploadOrRefresh(pSelf, graphics);

    // Bind the texture.
    graphics->BindTexture(graphics, pSelf->private.TextureGpuHandle);

    // Draw.
    if(pSelf->pNumIndices > 0 && pSelf->pIndices != NULL) {
        vec3 zero = (vec3){0.f, 0.f, 0.f};
        vec3 one = (vec3){1.f, 1.f, 1.f};
        graphics->DrawFromVBOTranslationIndices(
            graphics,
            pSelf->private.MeshGpuHandle,
            pSelf->pNumVertices,
            zero, zero, one,
            pSelf->pIndices, pSelf->pNumIndices
        );
    }
    else graphics->DrawFromVBO(graphics, pSelf->private.MeshGpuHandle, pSelf->pNumVertices);
}

void TestMesh_DestroySelf(SELF) {
    if(pSelf != NULL && pSelf->pVertices != NULL) {
        free(pSelf->pVertices);
        pSelf->pNumVertices = 0;
    }
    if(pSelf != NULL && pSelf->pIndices != NULL) {
        free(pSelf->pIndices);
        pSelf->pNumIndices = 0;
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

#define STR_VALUE(arg) #arg
#define cgltf_primtype_tostr(PRIMTYPE, STORAGE)\
switch(PRIMTYPE) {\
    case cgltf_primitive_type_points:                   break;\
	case cgltf_primitive_type_lines:                   STORAGE = STR_VALUE(cgltf_primitive_type_lines); break;\
	case cgltf_primitive_type_line_loop:               STORAGE = STR_VALUE(PRIMcgltf_primitive_type_line_loopTYPE); break;\
	case cgltf_primitive_type_line_strip:              STORAGE = STR_VALUE(cgltf_primitive_type_line_strip); break;\
	case cgltf_primitive_type_triangles:               STORAGE = STR_VALUE(cgltf_primitive_type_triangles); break;\
	case cgltf_primitive_type_triangle_strip:          STORAGE = STR_VALUE(cgltf_primitive_type_triangle_strip); break;\
	case cgltf_primitive_type_triangle_fan:            STORAGE = STR_VALUE(cgltf_primitive_type_triangle_fan); break;\
    default:                                           STORAGE = "UNKNOWN PRIM!";     break;\
}\

#define cgltf_accessor_type_tostr(AT, STORAGE)\
switch((AT)){\
	case cgltf_type_scalar:          (STORAGE) = STR_VALUE(cgltf_type_scalar);      break;\
	case cgltf_type_vec2:            (STORAGE) = STR_VALUE(cgltf_type_vec2);        break;\
	case cgltf_type_vec3:            (STORAGE) = STR_VALUE(cgltf_type_vec3);        break;\
	case cgltf_type_vec4:            (STORAGE) = STR_VALUE(cgltf_type_vec4);        break;\
	case cgltf_type_mat2:            (STORAGE) = STR_VALUE(cgltf_type_mat2);        break;\
	case cgltf_type_mat3:            (STORAGE) = STR_VALUE(cgltf_type_mat3);        break;\
	case cgltf_type_mat4:            (STORAGE) = STR_VALUE(cgltf_type_mat4);        break;\
    default:\
    case cgltf_type_invalid:         (STORAGE) = STR_VALUE(cgltf_type_invalid);     break;\
}\

#define cgltf_component_type_tostr(AT, STORAGE)\
switch((AT)){\
    case cgltf_component_type_invalid:                      (STORAGE) = STR_VALUE(cgltf_component_type_invalid);      break;\
	case cgltf_component_type_r_8: /* BYTE */               (STORAGE) = STR_VALUE(cgltf_component_type_r_8);          break;\
	case cgltf_component_type_r_8u: /* UNSIGNED_BYTE */     (STORAGE) = STR_VALUE(cgltf_component_type_r_8u);         break;\
	case cgltf_component_type_r_16: /* SHORT */             (STORAGE) = STR_VALUE(cgltf_component_type_r_16);         break;\
	case cgltf_component_type_r_16u: /* UNSIGNED_SHORT */   (STORAGE) = STR_VALUE(cgltf_component_type_r_16u);        break;\
	case cgltf_component_type_r_32u: /* UNSIGNED_INT */     (STORAGE) = STR_VALUE(cgltf_component_type_r_32u);        break;\
	case cgltf_component_type_r_32f: /* FLOAT */            (STORAGE) = STR_VALUE(cgltf_component_type_r_32f);         break;\
    case cgltf_component_type_max_enum:\
    default:                                                (STORAGE) = "UNKNOWN!";                                   break;\
}\

#define cgltf_attrib_type_tostr(AT, STORAGE)\
switch((AT)){\
    case cgltf_attribute_type_invalid:          (STORAGE) = STR_VALUE(cgltf_attribute_type_invalid);     break;\
    case cgltf_attribute_type_position:         (STORAGE) = STR_VALUE(cgltf_attribute_type_position);    break;\
    case cgltf_attribute_type_normal:           (STORAGE) = STR_VALUE(cgltf_attribute_type_normal);      break;\
    case cgltf_attribute_type_tangent:          (STORAGE) = STR_VALUE(cgltf_attribute_type_tangent);     break;\
    case cgltf_attribute_type_texcoord:         (STORAGE) = STR_VALUE(cgltf_attribute_type_texcoord);    break;\
    case cgltf_attribute_type_color:            (STORAGE) = STR_VALUE(cgltf_attribute_type_color);       break;\
    case cgltf_attribute_type_joints:           (STORAGE) = STR_VALUE(cgltf_attribute_type_joints);      break;\
    case cgltf_attribute_type_weights:          (STORAGE) = STR_VALUE(cgltf_attribute_type_weights);     break;\
    case cgltf_attribute_type_custom:           (STORAGE) = STR_VALUE(cgltf_attribute_type_custom);      break;\
    case cgltf_attribute_type_max_enum:         (STORAGE) = STR_VALUE(cgltf_attribute_type_max_enum);    break;\
    default:                                    (STORAGE) = "UNKNOWN!";                                  break;\
}\

int TestMesh_private_ReadMeshAt(SELF, cgltf_data* data, uint32_t index) {
    if(data == NULL) return -1;

    cgltf_mesh meshAt = data->meshes[index];

    // Iterate primitives
    pSelf->Log(pSelf, "primitives count: %d", meshAt.primitives_count);
    for(int indexMesh = 0; indexMesh < meshAt.primitives_count; indexMesh++) {
        cgltf_primitive* prim = meshAt.primitives + indexMesh;

        char* primTypeStr = NULL;
        cgltf_primtype_tostr(prim->type, primTypeStr);
        pSelf->Log(pSelf, 
            "\tprimitive: %s (type=%d)", 
            primTypeStr,
            prim->type
        );

        /* ================= Indices =================== */
        const char* indicesBlobType = NULL;
        cgltf_accessor *indicesBlob = prim->indices;
        cgltf_accessor_type_tostr(indicesBlob->type, indicesBlobType);

        pSelf->Log(pSelf, "Indices: %u, (type-%s)", indicesBlob->count, indicesBlobType);

        // Alloc indices
        if(pSelf->pIndices == NULL)     pSelf->pIndices = malloc(sizeof(uint32_t) * indicesBlob->count);
        else                            pSelf->pIndices = realloc(pSelf->pIndices, sizeof(uint32_t) * indicesBlob->count);
        pSelf->pNumIndices = indicesBlob->count;

        // Validate indices
        for(int testIndices = 0; testIndices < pSelf->pNumIndices; testIndices++) {
            // Read in indices
            cgltf_accessor_read_uint(indicesBlob, testIndices, &pSelf->pIndices[testIndices], 1);
            pSelf->Log(pSelf, "\t\tIndex[%d]: %u", testIndices, pSelf->pIndices[testIndices]);
        }
        /* ================= Indices =================== */

        /* ================= Vertices =================== */
        int peekedVerticesCount = prim->attributes[0].data->count;
        pSelf->Log(pSelf, "Prim Attributes Count: %d", prim->attributes_count);
        pSelf->Log(pSelf, "Peek Vertex Count: %d", peekedVerticesCount);

        // Allocate vertices in advanced.
        if(pSelf != NULL)       pSelf->pVertices = realloc(pSelf->pVertices, sizeof(Vertex) * peekedVerticesCount);
        else                    pSelf->pVertices = malloc(sizeof(Vertex) * peekedVerticesCount);

        memset(pSelf->pVertices, 0, sizeof(Vertex) * peekedVerticesCount);
        pSelf->pNumVertices = peekedVerticesCount;
        

        // Iterate attributes (POSITION, TEXCOORD0, etc.)
        for(int indexPrimAttrib = 0; indexPrimAttrib < prim->attributes_count; indexPrimAttrib++) {

            // Grab cgltf attribute from the primitive.
            cgltf_attribute* attrib = &prim->attributes[indexPrimAttrib];
            char *attribTypeStr = NULL;
            cgltf_attrib_type_tostr(attrib->type, attribTypeStr);

            // Grab the accessor that will let us get at the data ***hopefully*** cleanly.
            cgltf_accessor* attribDataBlob = attrib->data;
            char* accStr = NULL;
            cgltf_accessor_type_tostr(attribDataBlob->type, accStr);
            pSelf->Log(pSelf, "\t\taccessor: %s - %d (%s) (sparse=%d)", attribDataBlob->name, attribDataBlob->type, accStr, attribDataBlob->is_sparse);
            
            pSelf->Log(pSelf,
                "\t\tattrib: %s (index=%d, data=%p (count: %d), indexPrimAttrib type='%s'(%d))",
                attrib->name,
                attrib->index,
                attrib->data,
                attrib->data != NULL ? attrib->data->count : 0,
                attribTypeStr,
                attrib->type
            );

            // Vertices already allocated. Let's iterate the attributes (POS, TEXCOORD, ETC.) and fill the values.
            for(int iAttrib = 0; iAttrib < attribDataBlob->count; iAttrib++) {

                if(iAttrib > pSelf->pNumVertices) {
                    pSelf->Log(pSelf, "\t\tWARNING: iAttrib is greater than pSelf->pNumVertices (%d > %d)", iAttrib, pSelf->pNumVertices);
                    continue;
                }

                // VERTEX DATA BEING READ IN HERE.
                switch(attribDataBlob->type) {
                    case cgltf_type_vec3:
                        if(attrib->type != cgltf_attribute_type_position) break;

                        cgltf_accessor_read_float(attribDataBlob, iAttrib, pSelf->pVertices[iAttrib].position, 3);
                        break;
                    default: break;
                }
            }
            
            /*
            // Juicy now.
            size_t readVertexBufferSize = sizeof(Vertex) * attribDataBlob->count;
            Vertex* readVertices = malloc(readVertexBufferSize);
            memset(readVertices, 0, readVertexBufferSize);

            pSelf->Log(pSelf, "\tAllocated Vertex Buffer: %u", readVertexBufferSize);

            size_t attribBlobCnt = attribDataBlob->count;
            pSelf->Log(pSelf, "\t\tAttribBlobCount=%u (isSparse: %d)", attribBlobCnt, attribDataBlob->is_sparse);
            for(size_t iAttrib = 0; iAttrib < attribBlobCnt; iAttrib++) {
                vec3 tempV = {0};
                switch(attribDataBlob->type) {
                    case cgltf_type_vec3:
                        if(attrib->type != cgltf_attribute_type_position) break;
                        
                        // Read directly into that vertex!
                        cgltf_accessor_read_float(attribDataBlob, iAttrib, readVertices[iAttrib].position, 3);

                        pSelf->Log(pSelf,
                            "\t\t\t[%d] Vec3=(%.2f, %.2f, %.2f)", iAttrib,
                            readVertices[iAttrib].x, readVertices[iAttrib].y, readVertices[iAttrib].z
                        );
                        break;
                    case cgltf_type_vec2:
                        if(attrib->type != cgltf_attribute_type_texcoord) break;
                        break;
                    default:
                        break;
                }
            }


            if(pSelf->pVertices != NULL)
                free(pSelf->pVertices);
            
            pSelf->pVertices = readVertices;
            pSelf->pNumVertices = attribBlobCnt;
            pSelf->pHasChanged = 1;
            */
        }

        // Peek vertices
        pSelf->Log(pSelf, "==== checking vertices ====");
        for (int peekVertices = 0; peekVertices < pSelf->pNumVertices; peekVertices++)
        {
            pSelf->Log(pSelf, "Vertex[%d] = Vec3(%.2f, %.2f, %.2f)",
                       peekVertices,
                       pSelf->pVertices[peekVertices].x,
                       pSelf->pVertices[peekVertices].y,
                       pSelf->pVertices[peekVertices].z);
        }
        pSelf->Log(pSelf, "==== done checking vertices ====");
        /* ================= Vertices =================== */
    }

    return cgltf_result_success;
}

#define cgltf_result_tostr(RESULT, STORAGE)\
switch((RESULT)){\
    case cgltf_result_success:            (STORAGE) = "cgltf_result_success";            break;\
	case cgltf_result_data_too_short:     (STORAGE) = "cgltf_result_data_too_short";     break;\
	case cgltf_result_unknown_format:     (STORAGE) = "cgltf_result_unknown_format";     break;\
	case cgltf_result_invalid_json:       (STORAGE) = "cgltf_result_invalid_json";       break;\
	case cgltf_result_invalid_gltf:       (STORAGE) = "cgltf_result_invalid_gltf";       break;\
	case cgltf_result_invalid_options:    (STORAGE) = "cgltf_result_invalid_options";    break;\
	case cgltf_result_file_not_found:     (STORAGE) = "cgltf_result_file_not_found";     break;\
	case cgltf_result_io_error:           (STORAGE) = "cgltf_result_io_error";           break;\
	case cgltf_result_out_of_memory:      (STORAGE) = "cgltf_result_out_of_memory";      break;\
	case cgltf_result_legacy_gltf:        (STORAGE) = "cgltf_result_legacy_gltf";        break;\
    default:                              (STORAGE) = "UNKNOWN!";                                 break;\
}\

int TestMesh_ReadGLTFAtPath(SELF, const char* path) {
    cgltf_options options = {0};
    cgltf_data* data = NULL;
    cgltf_result result = cgltf_parse_file(&options, path, &data);
    result = cgltf_load_buffers(&options, data, path);

    if (result == cgltf_result_success)
    {
        size_t meshCount = data->meshes_count;
        pSelf->Log(pSelf, "Mesh Count: %u", meshCount);
        if(meshCount > 0) {
            TestMesh_private_ReadMeshAt(pSelf, data, 0);
        }
        
        // data->meshes
	    cgltf_free(data);
        return cgltf_result_success;
    }
    else {

        char *resultMsg = NULL;
        cgltf_result_tostr(result, resultMsg);

        pSelf->Log(pSelf, "Failed to load asset at '%s'. Result Type: %d (%s)", path, result, resultMsg);
        
        return result;
    }
}

#undef SELF