#version 460

layout (location = 0) in vec3 vertexIn;

out vec3 localPos;

uniform mat4 view;
uniform mat4 proj;

void main()
{
	localPos = vertexIn;
	gl_Position = proj * view * vec4(vertexIn, 1.0);
}
