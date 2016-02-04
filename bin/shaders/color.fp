#version 120

varying vec3 Position;
uniform vec3 Brightness;
uniform vec3 MaterialDiffuse;

void main()
{
    gl_FragColor = vec4(mix(vec4(MaterialDiffuse,1.0), vec4(0.0, 0.0, 0.0, 1.0), abs(Position.z/100.0)).xyz, 1.0);
}

