#version 100
#define P lowp

/*varying P vec3 VertexPosition;*/
varying P vec3 Position;
varying P vec2 Wrap;
/*varying P vec2 Normal;*/

uniform sampler2D Texture;
uniform P float Fade;

// This color key stuff could be done on the CPU, and using a separate tex
/*uniform P vec4 ColorKeyLow;*/
/*uniform P vec4 ColorKeyHigh;*/
/*uniform P vec4 ColorReplaceLow;*/
/*uniform P vec4 ColorReplaceHigh;*/

bool floatcmp(P float a, P float b, P float e)
{
    return abs(a-b) < e;
}

void main()
{
    P vec4 color = texture2D(Texture, Wrap);
    P float e = 0.1; // threshold
    if(floatcmp(color.r, 1.0, e) &&
        floatcmp(color.g, 0.0, e) &&
        floatcmp(color.b, 1.0, e))
    {
        discard;
    }
    if(floatcmp(color.a, 0.0, e))
        discard;
    gl_FragColor = color * Fade;
}

