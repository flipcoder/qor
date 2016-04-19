#version 120
#define MAX_LIGHTS 8

uniform vec4 FogColor = vec4(0.0, 0.0, 0.0, 0.0);
uniform float Brightness = 1.0;
varying float Depth;

uniform int NumLights;
uniform vec3 LightAmbient[MAX_LIGHTS];
uniform vec3 LightDiffuse[MAX_LIGHTS];
uniform vec3 LightSpecular[MAX_LIGHTS];
uniform float LightDist[MAX_LIGHTS];
/*varying float LightDistV[MAX_LIGHTS];*/
varying vec3 LightDir[MAX_LIGHTS];
varying vec3 LightHalf[MAX_LIGHTS];

uniform sampler2D Texture;
uniform sampler2D TextureNrm;
uniform sampler2D TextureDisp;
uniform sampler2D TextureSpec;
uniform vec3 MaterialAmbient;
uniform vec4 MaterialDiffuse;
uniform vec3 MaterialSpecular;
uniform float MaterialShininess = 2.0;
/*uniform sampler2D TextureOcc;*/
/*uniform sampler2D TextureSpec;*/
/*uniform vec4 LightAmbient;*/
/*uniform vec3 LightAmbient;*/

/*uniform vec3 CameraPosition;*/

varying vec3 Position;
varying vec2 Wrap;
varying vec3 Eye;
/*varying vec3 Normal;*/
varying vec3 Tangent;
/*varying vec3 Bitangent;*/

#define M_PI 3.1415926535897932384626433832795
#define M_TAU (M_PI * 2.0)

void main(void)
{
    vec3 vVec = normalize(Eye);
    /*vec3 normal = normalize(Normal);*/
    
    float height = texture2D(TextureDisp, Wrap).r;
    height = height * 0.04 - 0.02;
    vec2 uvp = Wrap + (vVec.xy * height);
    
    vec4 base = texture2D(Texture, uvp);
    vec3 bump = normalize(2.0 * texture2D(TextureNrm, uvp).xyz - 1.0);
    float spec = texture2D(TextureSpec, uvp).r;
    /*float spec = 1.0f;*/
    
    vec4 fragcolor = vec4(0.0, 0.0, 0.0, 0.0);
    
    for(int i=0; i<NumLights; i++){
        
        float att = cos(clamp(length(LightDir[i])/LightDist[i],0.0,1.0) * M_TAU / 4.0);
        /*float att = 1.0f;*/
        /*float dist = length(LightDir[i]);*/
        /*float att = clamp(1.0 - 1.0/LightDist[i] * dist, 0.0, 1.0);*/
        vec3 lVec = normalize(LightDir[i]);
    
        vec4 vAmbient = vec4(LightAmbient[i],1.0);

        float diffuse = max(dot(lVec,bump),0.0);
        
        vec4 vDiffuse = vec4(LightDiffuse[i] * diffuse, 1.0);

        /*lVec = LightDir[i];*/
        float specular = pow(clamp(dot(reflect(-lVec, bump), vVec), 0.0, 1.0), 
                         MaterialShininess );

        /*float specular = pow(max(dot(LightHalf[i], bump), 0.0), MaterialShininess);*/
        
        vec4 vSpecular = vec4(LightSpecular[i] * specular,1.0);
        /*vec4 vSpecular = vec4(specular,specular,specular,1.0);*/
        
        fragcolor += att * (
            vec4(MaterialAmbient, MaterialDiffuse.a) * vAmbient*base +
            vec4(MaterialDiffuse.rgb, MaterialDiffuse.a) * vDiffuse*base +
            vec4(MaterialSpecular, MaterialDiffuse.a) * vSpecular*spec
        );
    }
    
    gl_FragColor = mix(fragcolor, vec4(FogColor.rgb,1.0), FogColor.a * Depth) * Brightness;
}

