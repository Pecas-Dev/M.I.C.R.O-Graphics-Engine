#version 460

in vec3 localPos;

out vec4 fragColor;

uniform samplerCube environmentMap;

const float PI = 3.14159265359;

void main()
{
	vec3 N = normalize(localPos);

	vec3 up = vec3(0.0, 1.0, 0.0);
	vec3 right = normalize(cross(up, N));
	up = normalize(cross(N, right));

	vec3 irradiance = vec3(0.0);
	float sampleCount = 0.0;

	const float delta = 0.025;

	for (float phi = 0.0; phi < 2.0 * PI; phi += delta)
	{
		for (float theta = 0.0; theta < 0.5 * PI; theta += delta)
		{
			vec3 tangentSample = vec3(sin(theta) * cos(phi), sin(theta) * sin(phi), cos(theta));
			vec3 sampleVector = tangentSample.x * right + tangentSample.y * up + tangentSample.z * N;

			irradiance += texture(environmentMap, sampleVector).rgb * cos(theta) * sin(theta);
			sampleCount += 1.0;
		}
	}

	irradiance = PI * irradiance / sampleCount;

	fragColor = vec4(irradiance, 1.0);
}
