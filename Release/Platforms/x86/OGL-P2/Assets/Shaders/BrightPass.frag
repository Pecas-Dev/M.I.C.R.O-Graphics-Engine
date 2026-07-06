#version 460

in vec2 uv;

out vec4 fragColor;

uniform sampler2D sceneTexture;
uniform float threshold;

void main()
{
	vec3 color = texture(sceneTexture, uv).rgb;

	float luminance = dot(color, vec3(0.2126, 0.7152, 0.0722));
	float contribution = max(luminance - threshold, 0.0) / max(luminance, 0.0001);

	fragColor = vec4(min(color * contribution, vec3(8.0)), 1.0);
}
