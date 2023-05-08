#version 420

in vec2 texCoord;
out vec4 fragColor;

// texture sampler
uniform sampler2D groundTex;

void main(void)
{
    fragColor = texture(groundTex, texCoord);
}
