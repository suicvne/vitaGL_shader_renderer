varying vec4 fragColor;
varying vec2 texCoord;

uniform sampler2D ourTexture;
uniform bool useTexture;

void main()
{
    if(useTexture)
        gl_FragColor = texture2D(ourTexture, texCoord) * fragColor;
    else
        gl_FragColor = fragColor;
}