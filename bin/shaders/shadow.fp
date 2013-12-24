#version 100

attribute vec3 VertexPosition;
varying vec3 Position;

void main()
{
    Position = VertexPosition;
    gl_Position = ModelViewProjection * vec4(VertexPosition, 1.0);
}

