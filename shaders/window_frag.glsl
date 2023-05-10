#version 420 core

vec3 Iamb = vec3(0.8, 0.8, 0.8);
vec3 ka = vec3(0.1, 0.1, 0.4);

uniform vec3 eyePos;
uniform samplerCube dynTex;

in vec4 fragWorldPos;
in vec3 fragWorldNor;

out vec4 fragColor;

void main(void)
{
	vec3 ambientColor = Iamb * ka;

	vec3 I = normalize(vec3(fragWorldPos) - eyePos);
    vec3 R = reflect(I, normalize(fragWorldNor));

	fragColor = vec4(0.6 * texture(dynTex, R).rgb + 0.4 * ambientColor, 1.0);
}
