#version 420 core

// All of the following variables could be defined in the OpenGL
// program and passed to this shader as uniform variables. This
// would be necessary if their values could change during runtim.
// However, we will not change them and therefore we define them 
// here for simplicity.

vec3 Iamb = vec3(0.8, 0.8, 0.8); // ambient light intensity
vec3 ka = vec3(0.1, 0.5, 0.1);   // ambient reflectance coefficient

uniform vec3 eyePos;
uniform samplerCube dynTex;

in vec4 fragWorldPos;
in vec3 fragWorldNor;

out vec4 fragColor;

void main(void)
{
	// Compute lighting. We assume lightPos and eyePos are in world
	// coordinates. fragWorldPos and fragWorldNor are the interpolated
	// coordinates by the rasterizer.

	vec3 ambientColor = Iamb * ka;

	vec3 I = normalize(vec3(fragWorldPos) - eyePos);
    vec3 R = reflect(I, normalize(fragWorldNor));

	fragColor = vec4(0.3 * texture(dynTex, R).rgb + 0.7 * ambientColor, 1.0);
	fragColor = vec4(texture(dynTex, R).rgb, 1.0);
	//fragColor = texture(dynTex, vec3(fragWorldPos.x, fragWorldPos.y, fragWorldPos.z));
	//fragColor = vec4(fragWorldPos.x, fragWorldPos.y, fragWorldPos.z, 1);
	//fragColor = vec4(ambientColor, 1);
}
