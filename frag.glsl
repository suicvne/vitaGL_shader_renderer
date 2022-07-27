varying float lightIntensity;

varying vec4 fragColor;
varying vec2 texCoord;

uniform sampler2D ourTexture;
uniform bool useTexture;

void main()
{
    //if(fragColor.a < .5) discard;
    if(useTexture)
    {
        vec4 texel = texture2D(ourTexture, texCoord) * fragColor;
        //if(texel.a < .8) discard;
        //gl_FragColor = vec4((texel * 2.0 * 0.2).xyz, texel.a);
        gl_FragColor = texel;
    }
    else
        gl_FragColor = fragColor;
}