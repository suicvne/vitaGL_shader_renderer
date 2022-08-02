#include "mesh.h"
#include <memory.h>
#include <assert.h>

#define CGLTF_IMPLEMENTATION
#include "formats/cgltf.h"

#define SELF TeslaMesh* pSelf

TeslaMesh*   TestMesh_CreateHeap() {
    TeslaMesh* newMesh = malloc(sizeof(TeslaMesh));
    *newMesh = TestMesh_Create();

    return newMesh;
}

TeslaMesh TestMesh_Create() {
    return (TeslaMesh) {
        .pVertices = NULL,
        .pIndices  = NULL,
        .pNumIndices = 0,
        .pNumVertices = 0,
        .pHasChanged = 0,
        .pFreeAfterUpload = 0,

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
    if(pSelf->private.MeshGpuHandle == 0 || (pSelf->pHasChanged && !pSelf->pFreeAfterUpload)) {
        pSelf->pHasChanged = 0;
        graphics->Log(graphics, "Mesh is uploading data....");
        // No, we haven't. Let's do that!
        pSelf->private.MeshGpuHandle = 
            graphics->CreateVBOWithVertexData(graphics, (const float*)pSelf->pVertices, pSelf->pNumVertices);


        // Extreme optimization.
        if(pSelf->pFreeAfterUpload && pSelf->pVertices != NULL) {
            pSelf->Log(pSelf, "(Optimization) Freeing pVertices since they're uploaded to the GPU.");
            free(pSelf->pVertices);

            pSelf->pVertices = NULL;
        }
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
        vec3 zero = {0.f, 0.f, 0.f};
        vec3 one = {1.f, 1.f, 1.f};
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

#define MESH_CUBE_VB_LENGTH 24
const static Vertex pVertexBufferData[] = {
	{-1.00, -1.00, -1.00,		0.38, 0.38},
	{-1.00, -1.00, -1.00,		0.12, 0.12},
	{-1.00, -1.00, -1.00,		0.38, 0.38},
	{-1.00, -1.00, -1.00,		0.62, 0.62},
	{-1.00, -1.00, -1.00,		0.62, 0.62},
	{-1.00, -1.00, -1.00,		0.88, 0.88},
	{-1.00, -1.00, -1.00,		0.38, 0.38},
	{-1.00, -1.00, -1.00,		0.12, 0.12},
	{-1.00, -1.00, -1.00,		0.38, 0.38},
	{-1.00, -1.00, -1.00,		0.62, 0.62},
	{-1.00, -1.00, -1.00,		0.62, 0.62},
	{-1.00, -1.00, -1.00,		0.88, 0.88},
	{1.00, 1.00, 1.00,		0.38, 0.38},
	{1.00, 1.00, 1.00,		0.38, 0.38},
	{1.00, 1.00, 1.00,		0.38, 0.38},
	{1.00, 1.00, 1.00,		0.62, 0.62},
	{1.00, 1.00, 1.00,		0.62, 0.62},
	{1.00, 1.00, 1.00,		0.62, 0.62},
	{1.00, 1.00, 1.00,		0.38, 0.38},
	{1.00, 1.00, 1.00,		0.38, 0.38},
	{1.00, 1.00, 1.00,		0.38, 0.38},
	{1.00, 1.00, 1.00,		0.62, 0.62},
	{1.00, 1.00, 1.00,		0.62, 0.62},
	{1.00, 1.00, 1.00,		0.62, 0.62},
};

#define MESH_CUBE_IB_LENGTH 36
const static uint32_t pVertexBufferIndexData[] = {
	0,3,9,
	0,9,6,
	8,10,21,
	8,21,19,
	20,23,17,
	20,17,14,
	13,15,4,
	13,4,2,
	7,18,12,
	7,12,1,
	22,11,5,
	22,5,16,
};


// TODO: Rename this to default *****quad******
int TestMesh_InitWithDefaultCube(SELF) {

    // TODO: Optimizations to prevent re-initializing the default cube
    // vbo if it already exists.

    // Initailize with the data, 36 vertices.
    pSelf->InitWithVertices(pSelf, pVertexBufferData, MESH_CUBE_VB_LENGTH);

    // TODO: InitWithVerticesAndIndices() function

    // Realloc pIndices if necessary.
    if(pSelf->pIndices != NULL)     pSelf->pIndices = realloc(pSelf->pIndices, sizeof(uint32_t) * MESH_CUBE_IB_LENGTH);
    else                            pSelf->pIndices = malloc(sizeof(uint32_t) * MESH_CUBE_IB_LENGTH);

    // Copy default index data.
    memcpy(pSelf->pIndices, pVertexBufferIndexData, sizeof(uint32_t) * MESH_CUBE_IB_LENGTH);
    pSelf->pNumIndices =    MESH_CUBE_IB_LENGTH;

    pSelf->pHasChanged = 1;

    return 0;
}

/* ==================================== BEGIN CGLTF ======================================= */

#define STR_VALUE(arg) #arg
#define cgltf_primtype_tostr(PRIMTYPE, STORAGE)\
switch(PRIMTYPE) {\
    case cgltf_primitive_type_points:                   break;\
	case cgltf_primitive_type_lines:                   STORAGE = STR_VALUE(cgltf_primitive_type_lines); break;\
	case cgltf_primitive_type_line_loop:               STORAGE = STR_VALUE(cgltf_primitive_type_line_loop); break;\
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
    default:                                    (STORAGE) = "UNKNOWN!";                                  break;\
}\

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

// Just prints to stdout
void TestMesh_private_toCArr(SELF) {

    if(pSelf->pNumVertices > 100) return;

    pSelf->Log(pSelf, "const static Vertex pVertexBufferData[] = {");
    for(int i = 0; i < pSelf->pNumVertices; i++) {
        Vertex thisVert = pSelf->pVertices[i];
        pSelf->Log(pSelf, "\t{%.2f, %.2f, %.2f,\t\t%.2f, %.2f},",
            thisVert.x, thisVert.y, thisVert.z,
            thisVert.texCoordU, thisVert.texCoordV
        );
    }
    pSelf->Log(pSelf, "};");


    pSelf->Log(pSelf, "const static uint32_t pVertexBufferIndexData[] = {");
    for(int i = 0; i < pSelf->pNumIndices; (void)i) {

        pSelf->Log(pSelf, "\t%u,%u,%u,",
            pSelf->pIndices[i],
            pSelf->pIndices[i + 1],
            pSelf->pIndices[i + 2]
        );

        i += 3;
    }
    pSelf->Log(pSelf, "};");
}

/**
 * @brief O(too many)
 * 
 * @param data 
 * @param index 
 * @return int 
 */
int TestMesh_private_ReadMeshAt(SELF, cgltf_data* data, uint32_t index) {
    if(data == NULL) return -1;

    // Just grab the mesh specified instead of trying to iterate.
    // For now.....
    cgltf_mesh meshAt = data->meshes[index];

    // #ifdef MESH_PRINT_GLTF_DATA
    pSelf->Log(pSelf, "Mesh: %s", meshAt.name);
    pSelf->Log(pSelf, "\tprimitives count: %d", meshAt.primitives_count);
    // #endif

    // Iterate over the primitives in the mesh.
    for(int indexMesh = 0; indexMesh < meshAt.primitives_count; indexMesh++) {
        // Get prim type.
        cgltf_primitive* prim = meshAt.primitives + indexMesh;
        char* primTypeStr = NULL;
        cgltf_primtype_tostr(prim->type, primTypeStr);

        // Keep it simple by only supporting triangle prims for now.
        if(prim->type != cgltf_primitive_type_triangles)
        {
            pSelf->Log(pSelf, "\tWARNING: Skipping currently unsupported prim type '%s'", primTypeStr);
            continue;
        }

        // #ifdef MESH_PRINT_GLTF_DATA
        pSelf->Log(pSelf, 
            "\t\tprimitive: %s (type=%d)", 
            primTypeStr,
            prim->type
        );
        // #endif

        // First, let's grab the render indices for this mesh.
        /* ================= Indices =================== */
        const char* indicesBlobType = NULL;
        cgltf_accessor *indicesBlob = prim->indices;
        cgltf_accessor_type_tostr(indicesBlob->type, indicesBlobType);

        pSelf->Log(pSelf, "\t\tIndices: %u, (type-%s)", indicesBlob->count, indicesBlobType);

        // Alloc indices
        if(pSelf->pIndices == NULL)     pSelf->pIndices = malloc(sizeof(uint32_t) * indicesBlob->count);
        else                            pSelf->pIndices = realloc(pSelf->pIndices, sizeof(uint32_t) * indicesBlob->count);
        pSelf->pNumIndices = indicesBlob->count;

        
        // Validate indices
        for(int testIndices = 0; testIndices < pSelf->pNumIndices; testIndices++) {
            // Read in indices
            cgltf_accessor_read_uint(indicesBlob, testIndices, &pSelf->pIndices[testIndices], 1);
        #ifdef MESH_PRINT_GLTF_DATA_INDICES
            pSelf->Log(pSelf, "\t\t\tIndex[%d]: %u", testIndices, pSelf->pIndices[testIndices]);
        #endif
        }
        /* ================= Indices =================== */

        // Second, let's grab the vertex data from the primitive's attributes.
        /* ================= Vertices =================== */
        int peekedVerticesCount = prim->attributes[0].data->count;
        pSelf->Log(pSelf, "\t\tPrim Attributes Count: %d (Peeked Vertex Count: %d)", prim->attributes_count, peekedVerticesCount);

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
            pSelf->Log(pSelf, "\t\t\tattrib accessor: %s - %d (%s) (sparse=%d)", attribDataBlob->name, attribDataBlob->type, accStr, attribDataBlob->is_sparse);
            
            pSelf->Log(pSelf,
                "\t\t\t\tattrib: %s (index=%d, data=%p (count: %d), indexPrimAttrib type='%s'(%d))",
                attrib->name,
                attrib->index,
                attrib->data,
                attrib->data != NULL ? attrib->data->count : 0,
                attribTypeStr,
                attrib->type
            );

            // Vertices already allocated. Let's iterate the attributes (POS, TEXCOORD, ETC.) and fill the values.
            // So, we are iterating over the *values* in this attribute.
            for(int iAttrib = 0; iAttrib < attribDataBlob->count; iAttrib++) {

                if(iAttrib > pSelf->pNumVertices) {
                    pSelf->Log(pSelf, "\t\t\t\tWARNING: iAttrib is greater than pSelf->pNumVertices (%d > %d)", iAttrib, pSelf->pNumVertices);
                    continue;
                }

                // VERTEX DATA BEING READ IN HERE.
                switch(attribDataBlob->type) {
                    case cgltf_type_vec3:
                        if(attrib->type != cgltf_attribute_type_position) {
                            // pSelf->Log(pSelf, "\t\t\t\tUnhandled Attrib Type: %s which is type %s", attribTypeStr, accStr); 
                            break;
                        }
                        cgltf_accessor_read_float(attribDataBlob, iAttrib, pSelf->pVertices[iAttrib].position, 3);

                        // TODO: Normals.

                        break;
                    case cgltf_type_vec2:
                        if(attrib->type != cgltf_attribute_type_texcoord) break;
                        cgltf_accessor_read_float(attribDataBlob, iAttrib, pSelf->pVertices[iAttrib].uv, 2);
                        break;
                    default: break;
                }
            }
        }

        // Peek vertices
        #ifdef MESH_PRINT_GLTF_DATA_VERTICES
        pSelf->Log(pSelf, "==== checking vertices ====");
        for (int peekVertices = 0; peekVertices < pSelf->pNumVertices; peekVertices++)
        {
            pSelf->Log(pSelf, "Vertex[%d] = Vec3(%.2f, %.2f, %.2f)",
                       peekVertices,
                       pSelf->pVertices[peekVertices].x,
                       pSelf->pVertices[peekVertices].y,
                       pSelf->pVertices[peekVertices].z);
        }
        pSelf->Log(pSelf, "==== done checking vertices ====\n");
        #endif
        /* ================= Vertices =================== */
    }

    pSelf->Log(pSelf, "Loaded mesh %s.\n", meshAt.name);

    TestMesh_private_toCArr(pSelf);

    return cgltf_result_success;
}

int TestMesh_ReadGLTFAtPath(SELF, const char* path) {
    
    pSelf->Log(pSelf, "ReadGTLFAtPath");
    cgltf_options options = {0};
    cgltf_data* data = NULL;
    cgltf_result result = cgltf_parse_file(&options, path, &data);

    if (result == cgltf_result_success)
    {
        pSelf->Log(pSelf, "ReadGTLFAtPath parsed successfully!");
        result = cgltf_load_buffers(&options, data, path);
        if(result != cgltf_result_success) {

            pSelf->Log(pSelf, "ReadGTLFAtPath buffers parsed successfully!");
            char *resultMsg = NULL;
            cgltf_result_tostr(result, resultMsg);
            pSelf->Log(pSelf, "Failed to load asset BUFFERS from '%s'. Result Type: %d (%s)", result, resultMsg);

            // Prevent this from dangling.
            cgltf_free(data);
            return result;
        }

        size_t meshCount = data->meshes_count;
        if(meshCount > 0) {
            // Only read the first mesh for now.
            TestMesh_private_ReadMeshAt(pSelf, data, 0);
        }
        
        // Don't forget to free the data.
	    cgltf_free(data);

        // Our loading was a success!
        return cgltf_result_success;
    }
    else {

        // Failure, let's grab the reason and report it.
        char *resultMsg = NULL;
        cgltf_result_tostr(result, resultMsg);

        pSelf->Log(pSelf, "Failed to load asset at '%s'. Result Type: %d (%s)", path, result, resultMsg);
        
        return result;
    }
}

/* ==================================== END CGLTF ======================================= */

#undef SELF