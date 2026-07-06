#version 460

layout (location = 0) in vec3 vertexIn;

out vec3 direction;

uniform mat4 view; 
uniform mat4 proj;

void main()
{
	direction = vertexIn;

	vec4 position = proj * mat4(mat3(view)) * vec4(vertexIn, 1.0);
	gl_Position = position.xyww;
}
