#version 460

in vec3 vertexIn;

uniform mat4 model;
uniform mat4 lightSpaceMatrix;

void main()
{
	gl_Position = lightSpaceMatrix * model * vec4(vertexIn, 1.0);
}
