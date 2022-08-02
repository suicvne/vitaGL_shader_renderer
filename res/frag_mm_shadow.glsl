varying vec4 fragColor;
varying vec2 texCoord;

uniform sampler2D ourTexture;
uniform bool useTexture;

void main()
{
    gl_FragColor = vec4(.4, .4, .4, .58);
    // if(useTexture)
    //     gl_FragColor = texture2D(ourTexture, texCoord) * fragColor;
    // else
    //     gl_FragColor = fragColor;
}