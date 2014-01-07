#version 100

varying lowp vec3 Position;
varying lowp vec2 Wrap;

uniform sampler2D Texture;

bool floatcmp(lowp float a, lowp float b, lowp float e)
{
    return abs(a-b) < e;
}

void main()
{
    lowp vec4 color = texture2D(Texture, Wrap);

    lowp float e = 0.1;
    if(floatcmp(color.r, 1.0, e) &&
        floatcmp(color.g, 0.0, e) &&
        floatcmp(color.b, 1.0, e))
    {
        discard;
    }
    if(floatcmp(color.a, 0.0, e))
        discard;
    gl_FragColor = color;
}

