#version 120

/*varying vec3 VertexPosition;*/
varying vec3 Position;
varying vec2 Wrap;
varying vec3 Normal;
varying vec3 LightDir;
varying vec3 Eye;

uniform sampler2D Texture;
uniform vec3 LightAmbient;
uniform vec3 LightDiffuse;
uniform vec3 LightAtten;
/*uniform mat4 NormalMatrix;*/

uniform vec4 MaterialAmbient;
uniform vec4 MaterialDiffuse;
uniform vec4 MaterialSpecular;

#define M_PI 3.1415926535897932384626433832795
#define M_TAU (M_PI * 2.0)

bool floatcmp(float a, float b, float e)
{
    return abs(a-b) < e;
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

    float dist = length(LightDir);
    float atten = 1.0 / (LightAtten.x + LightAtten.y * dist + LightAtten.z * dist * dist);
    
    vec3 NormalN = normalize(Normal);
    vec3 LightDirN = normalize(LightDir);
    vec3 EyeN = normalize(Eye);
    float mag = max(dot(NormalN,LightDirN), 0.0);
    
    /*gl_FragColor = vec4(LightDirN.xyz, 1.0);*/
    /*gl_FragColor = vec4(mag,mag,mag,1.0);*/
    gl_FragColor = max(atten * mag * vec4(LightDiffuse,1.0) * color, 0.0);
    /*gl_FragColor = color * vec4(LightAmbient,1.0) * vec4(LightDiffuse,1.0);*/
    /*gl_FragColor = color;*/
    /*gl_FragColor = vec4(1.0, 0.0, 0.0, 1.0);*/
}

