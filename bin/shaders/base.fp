#version 120

varying vec3 Position;
varying vec2 Wrap;

uniform sampler2D Texture;
/*uniform vec4 LightAmbient = vec4(1, 1, 1, 1);*/

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
    gl_FragColor = color;
}

