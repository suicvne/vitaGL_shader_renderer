#ifndef __MUL_DEF_H__
#define __MUL_DEF_H__

static const char* Mul_private_DefaultShaders = 
    "//\n//  Shaders.metal\n//  MetalDemo\n//\n//  Created by Warren Moore on 10/28/14.\n//  Copyright (c) 2014 objc.io. All rights reserved.\n//\n\n#include <metal_stdlib>\n#include <simd/simd.h>\n\nusing namespace metal;\n\ntypedef struct\n{\n    float4x4 rotation_matrix;\n} Uniforms;\n\ntypedef struct\n{\n    float4 position;\n    float4 color;\n} VertexIn;\n\ntypedef struct {\n    float4 position [[position]];\n    half4  color;\n} VertexOut;\n\nvertex VertexOut vertex_function(device VertexIn *vertices [[buffer(0)]],\n                                 constant Uniforms &uniforms [[buffer(1)]],\n                                 uint vid [[vertex_id]])\n{\n    VertexOut out;\n    out.position = uniforms.rotation_matrix * vertices[vid].position;\n    out.color = half4(vertices[vid].color);\n    return out;\n}\n\nfragment half4 fragment_function(VertexOut in [[stage_in]])\n{\n    return in.color;\n}";

#endif