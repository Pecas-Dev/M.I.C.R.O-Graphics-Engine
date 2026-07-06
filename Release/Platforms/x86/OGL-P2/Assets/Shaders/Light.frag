#version 460

struct Light
{
	vec3 position;
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;

	float intensity;
	float constantAtt;
	float linearAtt;
	float quadraticAtt;
};

struct Material
{
	float shininess;
	float roughness;
	float metallic;
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
};

in vec4 colorOut;
in vec3 vertexOut;
in vec2 textureOut;
in vec3 normalOut;
in mat3 TBN;
in vec4 lightSpacePosition;

out vec4 fragColor;

uniform Light light;
uniform Material material;
uniform vec3 cameraPosition;

uniform bool isTextured;
uniform bool hasNormalMap;
uniform bool hasRoughnessMap;
uniform bool hasMetallicMap;
uniform bool hasShadows;
uniform bool hasIBL;

uniform float iblIntensity;

uniform sampler2D textureImage;
uniform sampler2D normalMap;
uniform sampler2D roughnessMap;
uniform sampler2D metallicMap;
uniform sampler2D shadowMap;

uniform samplerCube irradianceMap;
uniform samplerCube prefilterMap;
uniform sampler2D brdfLUT;

const float PI = 3.14159265359;

float CalculateShadow(vec3 N, vec3 L)
{
	vec3 projected = lightSpacePosition.xyz / lightSpacePosition.w;
	projected = projected * 0.5 + 0.5;

	if (projected.z > 1.0) { return 0.0; }

	float bias = max(0.003 * (1.0 - dot(N, L)), 0.0005);

	float shadow = 0.0;
	vec2 texelSize = 1.0 / textureSize(shadowMap, 0);

	for (int x = -1; x <= 1; x++)
	{
		for (int y = -1; y <= 1; y++)
		{
			float depth = texture(shadowMap, projected.xy + vec2(x, y) * texelSize).r;
			shadow += (projected.z - bias > depth) ? 1.0 : 0.0;
		}
	}

	return shadow / 9.0;
}

void main()
{
	vec3 albedo = material.diffuse;
	float alpha = 1.0;

	if (isTextured)
	{
		vec4 textureColor = texture(textureImage, textureOut);
		albedo *= pow(textureColor.rgb, vec3(2.2));
		alpha = textureColor.a;
	}

	vec3 N = normalize(normalOut);

	if (hasNormalMap)
	{
		N = normalize(TBN * (texture(normalMap, textureOut).rgb * 2.0 - 1.0));
	}

	float roughness = hasRoughnessMap ? texture(roughnessMap, textureOut).g : material.roughness;
	roughness = clamp(roughness, 0.045, 1.0);

	float metallic = hasMetallicMap ? texture(metallicMap, textureOut).b : material.metallic;
	metallic = clamp(metallic, 0.0, 1.0);

	vec3 V = normalize(cameraPosition - vertexOut);
	vec3 L = normalize(light.position - vertexOut);
	vec3 H = normalize(V + L);

	float lightDistance = length(light.position - vertexOut);
	float attenuation = 1.0 / (light.constantAtt + light.linearAtt * lightDistance + light.quadraticAtt * lightDistance * lightDistance);
	vec3 radiance = light.diffuse * light.intensity * attenuation;

	float NdotL = max(dot(N, L), 0.0);
	float NdotV = max(dot(N, V), 0.0);
	float NdotH = max(dot(N, H), 0.0);
	float HdotV = max(dot(H, V), 0.0);

	float a = roughness * roughness;
	float a2 = a * a;
	float denominator = NdotH * NdotH * (a2 - 1.0) + 1.0;
	float D = a2 / (PI * denominator * denominator);

	float k = (roughness + 1.0) * (roughness + 1.0) / 8.0;
	float G = (NdotV / (NdotV * (1.0 - k) + k)) * (NdotL / (NdotL * (1.0 - k) + k));

	vec3 F0 = mix(vec3(0.04), albedo, metallic);
	vec3 F = F0 + (1.0 - F0) * pow(1.0 - HdotV, 5.0);

	vec3 specular = (D * G * F) / max(4.0 * NdotV * NdotL, 0.0001);

	vec3 kD = (vec3(1.0) - F) * (1.0 - metallic);

	float shadow = hasShadows ? CalculateShadow(N, L) : 0.0;

	vec3 directLight = (kD * albedo / PI + specular) * radiance * NdotL * (1.0 - shadow);

	vec3 ambientLight;

	if (hasIBL)
	{
		vec3 R = reflect(-V, N);

		vec3 F_ibl = F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(1.0 - NdotV, 5.0);
		vec3 kD_ibl = (vec3(1.0) - F_ibl) * (1.0 - metallic);

		vec3 irradiance = texture(irradianceMap, N).rgb;
		vec3 diffuseIBL = irradiance * albedo;

		const float MAX_REFLECTION_LOD = 4.0;
		vec3 prefiltered = textureLod(prefilterMap, R, roughness * MAX_REFLECTION_LOD).rgb;
		vec2 brdf = texture(brdfLUT, vec2(NdotV, roughness)).rg;
		vec3 specularIBL = prefiltered * (F_ibl * brdf.x + brdf.y);

		ambientLight = (kD_ibl * diffuseIBL + specularIBL) * iblIntensity;
	}
	else
	{
		ambientLight = light.ambient * albedo;
	}

	fragColor = vec4(ambientLight + directLight, alpha);
}
