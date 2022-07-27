attribute vec3 vPosition;
attribute vec2 vTexCoord;
// attribute vec4 vColor;
varying vec4 fragColor;
varying vec2 texCoord;

uniform mat4 _mvp;
uniform vec4 vColor;

void main()
{
    gl_Position = _mvp * vec4(vPosition.xyz, 1.0);
    // gl_Position = vec4(vPosition.xyz, 1.0);
    fragColor = vColor;
    texCoord = vec2(vTexCoord.x, vTexCoord.y);
}