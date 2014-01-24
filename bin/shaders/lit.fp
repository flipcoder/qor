#version 120

uniform sampler2D Texture;
uniform sampler2D TextureNrm;
uniform sampler2D TextureDisp;
/*uniform sampler2D TextureOcc;*/
/*uniform sampler2D TextureSpec;*/

/*uniform vec3 CameraPosition;*/

varying vec3 Position;
varying vec2 Wrap;
varying vec3 Normal;

varying vec3 Tangent;
varying vec3 Bitangent;

varying vec3 ViewDir;

varying vec3 LightPos;
varying vec3 LightDir;

void main(void)
{
	vec3 eye = normalize(ViewDir);
	vec3 light = normalize(LightDir);
	float dist = length(LightPos);
	vec3 wlight = normalize(LightPos);
	
    float height = texture2D(TextureDisp, Wrap).r;
    height = height * 0.04 - 0.02;
    vec2 uvp = Wrap + (eye.xy * height);
	
	vec4 texel = texture2D(Texture, uvp);
    if(texel.a < 0.1)
        discard;
	vec3 bump = normalize(texture2D(TextureNrm, uvp).rgb * 2.0 - 1.0);
	
    float ambient = 0.1;
    /*float diffuse = 0.5;*/
    float diffuse = max(dot(light, bump), 0.0) * 0.5;
    float shine = 1.0 / 2.0;
    float spec = pow(clamp(dot(reflect(-eye, bump), light), 0.0, 1.0), shine);
	
    /*gl_FragColor = texel;*/
    gl_FragColor = vec4(
        texel.rgb * (ambient + diffuse + spec)
    , texel.a);
}

