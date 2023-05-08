#version 420

uniform mat4 viewingMatrix;
uniform mat4 modelingMatrix;
uniform mat4 projectionMatrix;

layout(location=0) in vec3 inVertex;
layout(location=1) in vec2 inTexCoord;

out vec2 texCoord;

void main(void)
{
    texCoord = inTexCoord; // texture coord equal to vertex position
    gl_Position = projectionMatrix * viewingMatrix * modelingMatrix * vec4(inVertex, 1);
    //vec4 pos = projectionMatrix * viewingMatrix * vec4(inVertex, 1);
    //gl_Position = pos.xyww;
}