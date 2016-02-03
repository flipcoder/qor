#version 120

/*varying vec3 VertexPosition;*/
varying vec3 Position;
varying vec2 Wrap;
/*varying vec2 Normal;*/

uniform sampler2D Texture;
uniform vec3 LightAmbient;

// This color key stuff could be done on the CPU, and using a separate tex
/*uniform vec4 ColorKeyLow;*/
/*uniform vec4 ColorKeyHigh;*/
/*uniform vec4 ColorReplaceLow;*/
/*uniform vec4 ColorReplaceHigh;*/

#define M_PI 3.1415926535897932384626433832795
#define M_TAU (M_PI * 2.0)

bool floatcmp(float a, float b, float e)
{
    return abs(a-b) < e;
}

bool colorcmp(vec4 a, vec4 b, float t)
{
    return floatcmp(a.r,b.r,t) &&
        floatcmp(a.g,b.g,t) &&
        floatcmp(a.b,b.b,t);
}

vec4 grayscale(vec4 c)
{
    float v = (c.r + c.g + c.b) / 3.0;
    return vec4(v,v,v, c.a);
}

float avg(vec3 c)
{
    return (c.r + c.g + c.b) / 3.0;
}

void main()
{
    vec4 color = texture2D(Texture, Wrap);
    float e = 0.1; // threshold
    if(floatcmp(color.r, 1.0, e) &&
        floatcmp(color.g, 0.0, e) &&
        floatcmp(color.b, 1.0, e))
    {
        discard;
    }
    if(floatcmp(color.a, 0.0, e)) {
        discard;
    }
    
    /*gl_FragColor = vec4(0.0, 0.0, 0.0, color.a);*/
    gl_FragColor = color * vec4(LightAmbient,1.0);
}

