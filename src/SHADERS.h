#ifndef _SHADERS_H_
#define _SHADERS_H_

#ifdef VITA

static const char vShaderString[] = 
    "void main(\n"
    " float2 aPosition,\n"
    " float2 vTexCoord,\n"
    " float4 vColor,\n"
    " uniform float4x4 mvp,\n"
    " uniform float4x4 _rot,\n"
    " uniform float4x4 _scale,\n"
    " float4 out vertPos : POSITION,\n"
    " float4 out fragColor : COLOR,\n"
    " float2 out texCoord : TEXCOORD0\n"
    ")\n"
    "{\n"
    "   float4x4 temp = mul(mvp, _rot);\n"
    "   temp = mul(temp, _scale);"
    "   vertPos = mul(temp, float4(aPosition, 1.f, 1.f));\n"
    "   fragColor = vColor;\n"
    "   texCoord = vTexCoord;\n"
    "}\n";

static const char vFragmentString[] =   
    "float4 main(\n"
      "float4 fragColor : COLOR,\n"
      "float2 texCoord : TEXCOORD0,\n"
      "uniform bool useTexture,\n"
      "uniform sampler2D ourTexture\n"
    ") : COLOR\n"
    "{\n"
    "   return float4(1.0f, 0.0f, 0.0f, 1.0f);\n"
    "}\n";

#else


/*
Note to future self:

GLSL requires that any variables be used at least once. 
 If not, you won't be able to get their attrib ID.
*/

static const char vShaderString[] = 
     "attribute vec2 vPosition;    \n"
     "attribute vec2 vTexCoord;    \n"
     "attribute vec4 vColor;      \n"
     "varying vec4 fragColor;           \n"
     "varying vec2 texCoord;            \n"
     "uniform mat4 mvp_mat;\n"
     "uniform mat4 _rot;\n"
     "uniform mat4 _scale;\n"
      "void main()                  \n"
      "{                            \n"
      "   gl_Position = mvp * _rot * _scale * vec4(vPosition.xy, 0, 1);  \n"
      "   fragColor = vColor;          \n"
      "   texCoord = vTexCoord;         \n"
      "}                            \n";

static const char vFragmentString[] =   
      "varying vec4 fragColor;\n"
      "varying vec2 texCoord;\n"
      "uniform sampler2D ourTexture;"
      "uniform bool useTexture;"
      "void main()                                  \n"
      "{                                            \n"
      " if(useTexture)\n"
      "     gl_FragColor = texture2D(ourTexture, texCoord) * fragColor;\n"
      " else\n"
      "     gl_FragColor = fragColor;\n"
      "}           ";

#endif

#endif