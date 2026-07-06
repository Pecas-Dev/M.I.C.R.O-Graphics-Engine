#version 460

in vec3 vertexIn;
in vec4 colorIn;
in vec2 textureIn;
in vec3 normalIn;
in vec3 tangentIn;

out vec3 vertexOut;
out vec4 colorOut;
out vec2 textureOut;
out vec3 normalOut;
out mat3 TBN;
out vec4 lightSpacePosition;

uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;
uniform mat3 normal;
uniform mat4 lightSpaceMatrix;

void main()
{
	colorOut = colorIn;
	textureOut = textureIn;
	normalOut = normalize(normal * normalIn);

	vec3 T = normal * tangentIn;
	vec3 N = normalOut;
	T = T - dot(T, N) * N;
	TBN = mat3(normalize(T), cross(N, normalize(T)), N);

	vertexOut = (model * vec4(vertexIn, 1.0)).xyz;
	lightSpacePosition = lightSpaceMatrix * vec4(vertexOut, 1.0);

	gl_Position = proj * view * model * vec4(vertexIn, 1.0);
}
