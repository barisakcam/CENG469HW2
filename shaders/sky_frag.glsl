#version 420

uniform samplerCube skyTex;

in vec3 texCoord;
out vec4 fragColor;

void main(void)
{
    fragColor = texture(skyTex, vec3(texCoord.x, texCoord.y, texCoord.z));
    //fragColor = texture(skyTex, texCoord.xyz);
    //fragColor = vec4(1., 1., 1., 1.);
}
