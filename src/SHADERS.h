#ifndef _SHADERS_H_
#define _SHADERS_H_

#ifdef VITA

static const char vShaderString[] = 
    "void main(\n"
    " float3 aPosition,\n"
    " float4 out vPosition: POSITION)\n"
    "{\n"
    "vPosition = float4(aPosition, 1.f);\n"
    "}\n";

static const char vFragmentString[] =   
    "float4 main(uniform float4 color) : COLOR\n"
    "{return float4(1.0f, 0.0f, 0.0f, 1.0f);}\n";

#else

static const char vShaderString[] = 
     "attribute vec4 vPosition;    \n"
     "attribute vec4 vColor;      \n"
     "varying vec4 fragColor;           \n"
     "uniform mat4 mvp;"
      "void main()                  \n"
      "{                            \n"
      "   gl_Position = mvp * vPosition;  \n"
      "   fragColor = vColor;          \n"
      "}                            \n";

static const char vFragmentString[] =   
      "varying vec4 fragColor;"
      "void main()                                  \n"
      "{                                            \n"
      "  gl_FragColor = fragColor;\n"
      "}           ";

#endif

#endif