#version 460

struct Light
{
	vec3 position;
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
};

struct Material
{
	float shininess;
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
};

in vec4 colorOut;
in vec3 vertexOut;
in vec2 textureOut;
in vec3 normalOut;

out vec4 fragColor;

uniform Light light;
uniform Material material;
uniform vec3 cameraPosition;

//uniform bool isLit;
uniform bool isTextured;
uniform sampler2D textureImage;

void main()
{
	vec3 ambientColor = light.ambient * material.ambient;

	vec3 normal = normalize(normalOut);
	vec3 lightDirection = normalize(light.position - vertexOut);
	float lightIntesity = max(dot(lightDirection, normal), 0.0);
	vec3 diffuseColor = light.diffuse * material.diffuse * lightIntesity;

	vec3 viewDirection = normalize(cameraPosition - vertexOut);
	vec3 reflection = reflect(-lightDirection, normal);
	float specularTerm = pow(max(dot(viewDirection, reflection), 0.0), material.shininess);
	vec3 specularColor = light.specular * material.specular * specularTerm;

	vec3 finalColor = ambientColor + diffuseColor + specularColor;

	if(isTextured)
	{
		fragColor = vec4(finalColor, 1.0) * texture(textureImage, textureOut);
	}

	else
	{
		fragColor = vec4(finalColor, 1.0);
	}

/*if(!isLit)
{}
else
{fragColor = colorOut;}*/
}