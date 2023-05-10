#version 420 core

uniform mat4 modelingMatrix;
uniform mat4 viewingMatrix;
uniform mat4 projectionMatrix;

layout(location=0) in vec3 inVertex;
layout(location=1) in vec3 inNormal;

out vec4 fragWorldPos;
out vec3 fragWorldNor;

void main(void)
{
	fragWorldNor = inverse(transpose(mat3x3(modelingMatrix))) * inNormal;

	fragWorldPos = modelingMatrix * vec4(inVertex, 1);
	
    gl_Position = projectionMatrix * viewingMatrix * modelingMatrix * vec4(inVertex, 1);
}

