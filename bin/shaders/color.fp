#version 120

/*varying vec3 VertexPosition;*/
varying vec3 Position;
varying vec2 Wrap;
/*varying vec2 Normal;*/
/*varying vec4 vLightAmbient;*/
uniform vec3 LightAmbient;
uniform vec3 MaterialAmbient;
uniform vec3 MaterialDiffuse;
uniform vec3 MaterialSpecular;

uniform sampler2D Texture;

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
    
    /*gl_FragColor = vec4(MaterialDiffuse.rgb, 1.0);*/
    /*gl_FragColor = color;*/
    gl_FragColor = vec4(mix(vec4(MaterialDiffuse,1.0), vec4(0.0, 0.0, 0.0, 1.0), abs(Position.z/100.0)).xyz, 1.0);
}

