attribute vec3 vPosition;
attribute vec2 vTexCoord;
attribute vec4 vColor;
varying vec4 fragColor;
varying vec2 texCoord;

uniform mat4 mvp;
uniform mat4 _rot;
uniform mat4 _scale;

void main()
{
    gl_Position = mvp * _rot * _scale * vec4(vPosition.xyz, 1);
    fragColor = vColor;
    texCoord = vTexCoord;
}