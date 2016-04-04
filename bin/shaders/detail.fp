#version 120

uniform sampler2D Texture;
uniform sampler2D TextureNrm;
uniform sampler2D TextureDisp;
uniform float MaterialShininess = 60.0;
/*uniform sampler2D TextureOcc;*/
/*uniform sampler2D TextureSpec;*/
/*uniform vec4 LightAmbient;*/
/*uniform vec3 LightAmbient;*/

/*uniform vec3 CameraPosition;*/

varying vec3 Position;
varying vec2 Wrap;
/*varying vec3 Normal;*/
/*varying vec3 Tangent;*/
/*varying vec3 Bitangent;*/

varying vec3 Eye;

varying vec3 LightPosT;
varying vec3 LightDir;

uniform vec3 LightDiffuse;
uniform vec3 LightSpecular;
uniform float LightDist;

#define M_PI 3.1415926535897932384626433832795
#define M_TAU (M_PI * 2.0)

void main(void)
{
    /*float distSqr = dot(LightDir, LightDir);*/
    float att = cos(clamp(length(LightDir)/LightDist,0.0,1.0) * M_TAU / 4.0);
    
    vec3 lVec = normalize(LightDir);
    vec3 vVec = normalize(Eye);
    
    /*float height = texture2D(TextureDisp, Wrap).r;*/
    /*height = height * 0.04 - 0.02;*/
    /*vec2 uvp = Wrap + (Eye.xy * height);*/
    
    vec4 base = texture2D(Texture, Wrap);
    
    vec3 bump = normalize( texture2D(TextureNrm, Wrap).xyz * 2.0 - 1.0);

    vec4 vAmbient = vec4(0.1,0.1,0.1,1.0);

    float diffuse = max( dot(lVec, bump), 0.0 );
    
    vec4 vDiffuse = vec4(LightDiffuse * diffuse, 1.0);

    float specular = pow(clamp(dot(reflect(-lVec, bump), vVec), 0.0, 1.0), 
                     MaterialShininess );
    
    vec4 vSpecular = vec4(LightSpecular * specular,1.0) * 0.1;
    
    gl_FragColor = att * (
        vAmbient*base +
        vDiffuse*base +
        vSpecular
    );
}

