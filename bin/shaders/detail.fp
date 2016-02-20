#version 120

uniform sampler2D Texture;
uniform sampler2D TextureNrm;
uniform sampler2D TextureDisp;
uniform float MaterialShininess = 100.0;
uniform float LightDist;
/*uniform sampler2D TextureOcc;*/
/*uniform sampler2D TextureSpec;*/
/*uniform vec4 LightAmbient;*/
/*uniform vec3 LightAmbient;*/

/*uniform vec3 CameraPosition;*/

varying vec3 Position;
varying vec2 Wrap;
varying vec3 Normal;

varying vec3 Tangent;
varying vec3 Bitangent;

varying vec3 Eye;

varying vec3 MVLightPos;
varying vec3 MVLightDir;

#define M_PI 3.1415926535897932384626433832795
#define M_TAU (M_PI * 2.0)

void main(void)
{
    vec3 eye = normalize(Eye);
    vec3 light = normalize(MVLightDir);
    float dist = length(MVLightPos - Position);
    vec3 wlight = normalize(MVLightPos);
    
    float height = texture2D(TextureDisp, Wrap).r;
    height = height * 0.04 - 0.02;
    vec2 uvp = Wrap + (eye.xy * height);
    
    float ambient = 0.1;
    vec4 texel = texture2D(Texture, Wrap);
    if(texel.a < 0.1)
        discard;
    vec3 bump = normalize(texture2D(TextureNrm, uvp).rgb * 2.0 - 1.0);
    
    float atten = cos(clamp(dist/LightDist,0.0,1.0) * M_TAU / 4.0);
    
    float diffuse = max(dot(light, bump), 0.0);
    /*float diffuse = max(dot(light, vec3(1.0)), 0.0);*/
    float spec = pow(clamp(dot(reflect(-light, bump), eye), 0.0, 1.0), MaterialShininess);
    
    /*[>gl_FragColor = texel;<]*/
    /*gl_FragColor = vec4(*/
    /*    (texel.rgb * (ambient + diffuse + spec))*/
    /*, texel.a);*/
    gl_FragColor = vec4(texel.rgb * diffuse + spec, 1.0);

    /*gl_FragColor = vec4(vec3((ambient + diffuse) * clamp(1.0 - dist / 50.0, 0.0, 1.0)), 1.0);*/
}

