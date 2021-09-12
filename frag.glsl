varying vec4 fragColor;
varying vec2 texCoord;

uniform sampler2D ourTexture;
uniform bool useTexture;

void main()
{
    if(fragColor.a < .5) discard;
    if(useTexture)
    {
        vec4 texel = texture2D(ourTexture, texCoord) * fragColor;
        if(texel.a < .8) discard;
        gl_FragColor = texel;
    }
    else
        gl_FragColor = fragColor;
}