#version 120

/*varying vec3 VertexPosition;*/
varying vec3 Position;
varying vec2 Wrap;
/*varying vec2 Normal;*/

uniform sampler2D Texture;
uniform vec4 LightAmbient = vec4(1, 1, 1, 1);

// This color key stuff could be done on the CPU, and using a separate tex
/*uniform vec4 ColorKeyLow;*/
/*uniform vec4 ColorKeyHigh;*/
/*uniform vec4 ColorReplaceLow;*/
/*uniform vec4 ColorReplaceHigh;*/

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
    
    /*gl_FragColor = color;*/
    gl_FragColor = color * LightAmbient;
    //gl_FragColor = color * LightAmbient * vec4(Position.xyz,1.0);
}

