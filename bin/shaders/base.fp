#version 120

uniform vec4 FogColor = vec4(0.0, 0.0, 0.0, 0.0);
uniform float Brightness = 1.0;
uniform float Ambient = 0.1;
varying vec3 Position;
varying vec2 Wrap;
/*varying vec3 Normal;*/
varying float Depth;
/*varying float Fade;*/

uniform sampler2D Texture;
uniform sampler2D TextureFade;
uniform vec4 MaterialEmissive = vec4(0.0, 0.0, 0.0, 0.0);

bool floatcmp(float a, float b, float e)
{
    return abs(a-b) < e;
}

void main()
{
    vec4 color = texture2D(Texture, Wrap);
    vec4 fade = texture2D(TextureFade, Wrap);
    /*color = mix(color, fade, Fade);*/

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
    vec4 fragcolor = mix(vec4(color.rgb * Ambient, color.a), vec4(color.rgb, color.a), MaterialEmissive.a);
    gl_FragColor = mix(fragcolor, vec4(FogColor.rgb,1.0), FogColor.a * Depth) * Brightness;
}

