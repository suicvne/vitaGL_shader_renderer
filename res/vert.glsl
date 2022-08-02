attribute vec3 vPosition;
attribute vec2 vTexCoord;
attribute vec4 vColor;
varying vec4 fragColor;
varying vec2 texCoord;
varying float lightIntensity;

uniform mat4 mvp;

//uniform mat4 _model;
//uniform mat4 _view;
// uniform mat4 _projection;

uniform mat4 _rot;
uniform mat4 _scale;
uniform vec4 lightDirection;

void main()
{
    gl_Position = mvp * vec4(vPosition.xyz, 1.0);
    fragColor = vColor;
    texCoord = vTexCoord;
}