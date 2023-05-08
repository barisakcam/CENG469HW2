#version 420

uniform samplerCube myCubeSampler;

in vec2 texCoord;
out vec4 fragColor;

// texture sampler
uniform sampler2D texture1;

void main(void)
{
    fragColor = texture(texture1, texCoord);
}
