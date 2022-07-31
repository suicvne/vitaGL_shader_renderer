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
    for(int i = 0; i < meshAt.primitives_count; i++) {
        cgltf_primitive* prim = meshAt.primitives + i;

        char* primTypeStr = NULL;
        cgltf_primtype_tostr(prim->type, primTypeStr);
        pSelf->Log(pSelf, 
            "\tprimitive: %s (type=%d)", 
            primTypeStr,
            prim->type
        );

        const char* indicesBlobType = NULL;
        cgltf_accessor *indicesBlob = prim->indices;
        cgltf_accessor_type_tostr(indicesBlob->type, indicesBlobType);

        pSelf->Log(pSelf, "Indices: %u, (type-%s)", indicesBlob->count, indicesBlobType);

        // Alloc indices
        //if(pSelf->pIndices != NULL) free(pSelf->pIndices);
        pSelf->pIndices =   malloc(sizeof(uint32_t) * pSelf->pNumIndices);
        pSelf->pNumIndices = indicesBlob->count;
        
        for(int indices = 0; indices < indicesBlob->count; indices++) {
            cgltf_uint indicesTest = 0;
            cgltf_accessor_read_uint(indicesBlob, indices, &indicesTest, 1);
            pSelf->pIndices[indices] = indicesTest;
            pSelf->Log(pSelf, "\tIndex[%d]: %u (SPARSE: %d)", indices, indicesTest, indicesBlob->is_sparse);
        }

        // Iterate attributes (POSITION, TEXCOORD0, etc.)
        for(int p = 0; p < prim->attributes_count; p++) {

            // Grab cgltf attribute from the primitive.
            cgltf_attribute* attrib = &prim->attributes[p];
            char *attribTypeStr = NULL;
            cgltf_attrib_type_tostr(attrib->type, attribTypeStr);

            // Grab the accessor that will let us get at the data ***hopefully*** cleanly.
            cgltf_accessor* attribBlob = attrib->data;
            char* accStr = NULL;
            cgltf_accessor_type_tostr(attribBlob->type, accStr);
            pSelf->Log(pSelf, "\t\taccessor: %s - %d (%s) (sparse=%d)", attribBlob->name, attribBlob->type, accStr, attribBlob->is_sparse);
            
            pSelf->Log(pSelf,
                "\t\tattrib: %s (index=%d, data=%p, type='%s'(%d))",
                attrib->name,
                attrib->index,
                attrib->data,
                attribTypeStr,
                attrib->type
            );
            
            // Juicy now.
            Vertex* readVertices = malloc(sizeof(Vertex) * attribBlob->count);
            memset(readVertices, 0, sizeof(Vertex) * attribBlob->count);

            size_t attribBlobCnt = attribBlob->count;
            pSelf->Log(pSelf, "\t\tAttribBlobCount=%u (isSparse: %d)", attribBlobCnt, attribBlob->is_sparse);
            for(size_t iAttrib = 0; iAttrib < attribBlobCnt; iAttrib++) {
                vec3 tempV = {0};
                switch(attribBlob->type) {
                    case cgltf_type_vec3:
                        if(attrib->type != cgltf_attribute_type_position) break;
                        
                        // Read directly into that vertex!
                        // cgltf_accessor_read_float(attribBlob, iAttrib, &readVertices[iAttrib], 5);
                        // assert(readVertices[iAttrib].position[0] == readVertices[iAttrib].x);

                        pSelf->Log(pSelf,
                            "\t\t\t[%d] Vec3=(%.2f, %.2f, %.2f)", iAttrib,
                            readVertices[iAttrib].x, readVertices[iAttrib].y, readVertices[iAttrib].z
                        );
                        break;
                    case cgltf_type_vec2:
                        if(attrib->type != cgltf_attribute_type_texcoord) break;

                        // Read directly in!!!!
                        // cgltf_accessor_read_float(attribBlob, iAttrib, readVertices[iAttrib].uv, 2);
                        /*
                        pSelf->Log(pSelf,
                            "\t\t\t[%d] Vec2=(%.2f, %.2f)", iAttrib,
                            readVertices[iAttrib].texCoordU, readVertices[iAttrib].texCoordV
                        );
                        */
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
        }
    }

    // Iterate data accessors.
    // These *must* be for like...? I have no idea actually.
    pSelf->Log(pSelf, "accessors count: %d", data->accessors_count);
    for (int i = 0; i < data->accessors_count; i++) {
        const cgltf_accessor* blob = data->accessors + i;
        char* accStr = NULL;
        cgltf_accessor_type_tostr(blob->type, accStr);
        pSelf->Log(pSelf, "accessor: %s - %d (%s)", blob->name, blob->type, accStr);
        
        if(blob->is_sparse) {
            pSelf->Log(pSelf, "\tSparse blob! Whatever that means LOL");
            size_t nfloats = cgltf_num_components(blob->type) * blob->count;
            cgltf_float* dense = (cgltf_float*)malloc(nfloats*sizeof(cgltf_float));
            if(cgltf_accessor_unpack_floats(blob, dense, nfloats) < nfloats) {
                pSelf->Log(pSelf, "\tWarning: Unable to fully unpack this sparse blob.");
                free(dense);
                return -1;
            }

            free(dense);
        }
        else{

            char* blobTypeStr = accStr;
            char* blobCompTypeStr = NULL;

            cgltf_component_type_tostr(blob->component_type, blobCompTypeStr);

            vec3 v = {0};
            switch(blob->component_type) {
                case cgltf_component_type_r_32f:
                    cgltf_accessor_read_float(blob, blob->offset + (i * blob->stride), v, 3);
                    pSelf->Log(pSelf, "\t%s - blob->component_type=%s;val=(%.2f, %.2f, %.2f)", blobTypeStr, blobCompTypeStr, v[0], v[1], v[2]);
                    break;
                default:break;
            }
        }
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