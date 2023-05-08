#version 420

uniform samplerCube myCubeSampler;

in vec3 texCoord;
out vec4 fragColor;

void main(void)
{
    fragColor = texture(myCubeSampler, texCoord);
}
