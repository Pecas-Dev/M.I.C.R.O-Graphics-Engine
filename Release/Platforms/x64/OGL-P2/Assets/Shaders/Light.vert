#version 460

in vec3 vertexIn;
in vec4 colorIn;
in vec2 textureIn;
in vec3 normalIn;

out vec3 vertexOut;
out vec4 colorOut;
out vec2 textureOut;
out vec3 normalOut;

uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;
uniform mat3 normal;

void main()
{
	colorOut = colorIn;
	textureOut = textureIn;
	normalOut = normalize(normal * normalIn);

	vertexOut = (model * vec4(vertexIn, 1.0)).xyz;

	gl_Position = proj * view * model * vec4(vertexIn, 1.0);
}