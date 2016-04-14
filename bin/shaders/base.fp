#version 120

uniform vec4 FogColor = vec4(0.0, 0.0, 0.0, 0.0);
uniform float Brightness = 1.0;
uniform float Ambient = 0.1;
varying vec3 Position;
varying vec2 Wrap;
/*varying vec3 Normal;*/
varying float Depth;

uniform sampler2D Texture;
uniform vec3 MaterialEmissive = vec3(0.0, 0.0, 0.0);

bool floatcmp(float a, float b, float e)
{
    return abs(a-b) < e;
}

void main()
{
    vec4 color = texture2D(Texture, Wrap);

    float e = 0.1;
    if(floatcmp(color.r, 1.0, e) &&
        floatcmp(color.g, 0.0, e) &&
        floatcmp(color.b, 1.0, e))
    {
        discard;
    }
    if(floatcmp(color.a, 0.0, e)) {
        discard;
    }
    vec4 fragcolor = vec4(color.rgb * Ambient, color.a) + vec4(MaterialEmissive,1.0);
    gl_FragColor = mix(fragcolor, vec4(FogColor.rgb,1.0), FogColor.a * Depth) * Brightness;
    
    /*gl_FragColor = vec4(Normal, 1.0);*/
}

