#version 460

in vec4 colorOut;
out vec4 fragColor;

void main()
{
	fragColor = vec4(pow(colorOut.rgb, vec3(2.2)), colorOut.a);
}
