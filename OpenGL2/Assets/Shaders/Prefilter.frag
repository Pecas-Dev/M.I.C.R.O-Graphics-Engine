#version 460

in vec3 localPos;

out vec4 fragColor;

uniform samplerCube environmentMap;
uniform float roughness;
uniform float sourceResolution;

const float PI = 3.14159265359;

float RadicalInverse_VdC(uint bits)
{
	bits = (bits << 16u) | (bits >> 16u);
	bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
	bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
	bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
	bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
	return float(bits) * 2.3283064365386963e-10;
}

vec2 Hammersley(uint i, uint count)
{
	return vec2(float(i) / float(count), RadicalInverse_VdC(i));
}

vec3 ImportanceSampleGGX(vec2 Xi, vec3 N, float roughness)
{
	float a = roughness * roughness;

	float phi = 2.0 * PI * Xi.x;
	float cosTheta = sqrt((1.0 - Xi.y) / (1.0 + (a * a - 1.0) * Xi.y));
	float sinTheta = sqrt(1.0 - cosTheta * cosTheta);

	vec3 H = vec3(cos(phi) * sinTheta, sin(phi) * sinTheta, cosTheta);

	vec3 up = abs(N.z) < 0.999 ? vec3(0.0, 0.0, 1.0) : vec3(1.0, 0.0, 0.0);
	vec3 tangent = normalize(cross(up, N));
	vec3 bitangent = cross(N, tangent);

	return normalize(tangent * H.x + bitangent * H.y + N * H.z);
}

void main()
{
	vec3 N = normalize(localPos);
	vec3 R = N;
	vec3 V = R;

	const uint SAMPLE_COUNT = 1024u;

	vec3 prefiltered = vec3(0.0);
	float totalWeight = 0.0;

	for (uint i = 0u; i < SAMPLE_COUNT; i++)
	{
		vec2 Xi = Hammersley(i, SAMPLE_COUNT);
		vec3 H = ImportanceSampleGGX(Xi, N, roughness);
		vec3 L = normalize(2.0 * dot(V, H) * H - V);

		float NdotL = max(dot(N, L), 0.0);

		if (NdotL > 0.0)
		{
			float D = ((roughness * roughness) * (roughness * roughness)) /
				(PI * pow(max(dot(N, H), 0.0) * max(dot(N, H), 0.0) * ((roughness * roughness) * (roughness * roughness) - 1.0) + 1.0, 2.0));
			float pdf = D * max(dot(N, H), 0.0) / (4.0 * max(dot(H, V), 0.0)) + 0.0001;

			float saTexel = 4.0 * PI / (6.0 * sourceResolution * sourceResolution);
			float saSample = 1.0 / (float(SAMPLE_COUNT) * pdf + 0.0001);

			float mipLevel = roughness == 0.0 ? 0.0 : 0.5 * log2(saSample / saTexel);

			prefiltered += textureLod(environmentMap, L, mipLevel).rgb * NdotL;
			totalWeight += NdotL;
		}
	}

	fragColor = vec4(prefiltered / max(totalWeight, 0.0001), 1.0);
}
