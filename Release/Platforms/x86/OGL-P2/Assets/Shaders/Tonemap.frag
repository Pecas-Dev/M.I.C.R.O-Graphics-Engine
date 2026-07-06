#version 460

in vec2 uv;

out vec4 fragColor;

uniform sampler2D sceneTexture;
uniform sampler2D bloomTexture;

uniform float exposure;
uniform float bloomStrength;
uniform bool bloomEnabled;

vec3 ACESFilm(vec3 x)
{
	return clamp((x * (2.51 * x + 0.03)) / (x * (2.43 * x + 0.59) + 0.14), 0.0, 1.0);
}

void main()
{
	vec4 scene = texture(sceneTexture, uv);
	vec3 color = scene.rgb;

	if (bloomEnabled)
	{
		color += texture(bloomTexture, uv).rgb * bloomStrength;
	}

	vec3 mapped = ACESFilm(color * exposure);
	mapped = pow(mapped, vec3(1.0 / 2.2));

	fragColor = vec4(mapped, 1.0);
}
