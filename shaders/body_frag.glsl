#version 420 core

// All of the following variables could be defined in the OpenGL
// program and passed to this shader as uniform variables. This
// would be necessary if their values could change during runtim.
// However, we will not change them and therefore we define them 
// here for simplicity.

vec3 Iamb = vec3(0.8, 0.8, 0.8); // ambient light intensity
vec3 ka = vec3(0.1, 0.5, 0.1);   // ambient reflectance coefficient

uniform vec3 eyePos;

in vec4 fragWorldPos;
in vec3 fragWorldNor;

out vec4 fragColor;

void main(void)
{
	// Compute lighting. We assume lightPos and eyePos are in world
	// coordinates. fragWorldPos and fragWorldNor are the interpolated
	// coordinates by the rasterizer.

	vec3 ambientColor = Iamb * ka;

	fragColor = vec4(ambientColor, 1);
}
