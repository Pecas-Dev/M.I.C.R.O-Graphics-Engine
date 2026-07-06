#version 460

in vec2 uv;

out vec4 fragColor;

uniform sampler2D sourceTexture;
uniform bool horizontal;

const float weights[5] = float[](0.227027, 0.1945946, 0.1216216, 0.054054, 0.016216);

void main()
{
	vec2 texelSize = 1.0 / textureSize(sourceTexture, 0);
	vec3 result = texture(sourceTexture, uv).rgb * weights[0];

	if (horizontal)
	{
		for (int i = 1; i < 5; i++)
		{
			result += texture(sourceTexture, uv + vec2(texelSize.x * i, 0.0)).rgb * weights[i];
			result += texture(sourceTexture, uv - vec2(texelSize.x * i, 0.0)).rgb * weights[i];
		}
	}
	else
	{
		for (int i = 1; i < 5; i++)
		{
			result += texture(sourceTexture, uv + vec2(0.0, texelSize.y * i)).rgb * weights[i];
			result += texture(sourceTexture, uv - vec2(0.0, texelSize.y * i)).rgb * weights[i];
		}
	}

	fragColor = vec4(result, 1.0);
}
