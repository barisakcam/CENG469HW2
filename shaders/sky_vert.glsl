#version 420

uniform mat4 viewingMatrix;
uniform mat4 modelingMatrix;
uniform mat4 projectionMatrix;

layout(location=0) in vec3 inVertex;

out vec3 texCoord;

void main(void)
{
    texCoord = inVertex * 1; // texture coord equal to vertex position
    gl_Position = projectionMatrix * viewingMatrix * modelingMatrix * vec4(inVertex, 1.0f);
    //vec4 pos = projectionMatrix * viewingMatrix * vec4(inVertex, 1.0f);
    //gl_Position = pos.xyww;
    //texCoord = vec3(inVertex.x, inVertex.y, -inVertex.z);
}