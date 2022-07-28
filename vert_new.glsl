attribute vec3 vPosition;
attribute vec2 vTexCoord;
varying vec4 fragColor;
varying vec2 texCoord;

uniform mat4 _model;
uniform mat4 _view;
uniform mat4 _projection;
uniform vec4 vColor;

void main()
{
    // gl_Position = _mvp * vec4(vPosition.xyz, 1.0);
    gl_Position = (_projection * _view * _model) * vec4(vPosition.xyz, 1.0);
    // gl_Position = (_projection * _view * _model) * vec4(vPosition.xyz, 1.0);
    fragColor = vColor;
    texCoord = vTexCoord;
}