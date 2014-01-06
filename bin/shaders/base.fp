#version 100

/*varying lowp vec3 VertexPosition;*/
varying lowp vec3 Position;
varying lowp vec2 Wrap;
/*varying lowp vec2 Normal;*/

uniform sampler2D Texture;
/*uniform lowp float Fade;*/

// This color key stuff could be done on the CPU, and using a separate tex
/*uniform lowp vec4 ColorKeyLow;*/
/*uniform lowp vec4 ColorKeyHigh;*/
/*uniform lowp vec4 ColorReplaceLow;*/
/*uniform lowp vec4 ColorReplaceHigh;*/

/*varying lowp vec4 FragColor;*/

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

