#version 460

in vec3 direction;

out vec4 fragColor;

uniform samplerCube environmentMap;
uniform float skyboxIntensity;

void main()
{
	vec3 color = texture(environmentMap, normalize(direction)).rgb * skyboxIntensity;

	fragColor = vec4(color, 1.0);
}
