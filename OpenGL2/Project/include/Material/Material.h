#pragma once

#include <Texture/Texture.h>
#include <Shader/Shader.h>

#include <glm.hpp>
#include <glad/gl.h>

#include <string>
#include <vector>


class Material
{
public:
	Material();

	const std::string& GetName() const;

	bool IsTextured() const;

	const Texture& GetNormalMap() const;
	const Texture& GetDiffuseMap() const;
	const Texture& GetSpecularMap() const;
	const Texture& GetAmbientMap() const;

	void SetShininess(GLfloat shininess);
	void SetAmbient(const glm::vec3& ambient);
	void SetDiffuse(const glm::vec3& diffuse);
	void SetSpecular(const glm::vec3& specular);

	bool Load(const std::string& filename, std::vector<Material>& materials);

	void SendToShader(const Shader& shader);

private:
	bool m_isTextured;
	std::string m_name;

	Texture m_normalMap;
	Texture m_diffuseMap;
	Texture m_specularMap;
	Texture m_ambientMap;

	GLfloat m_shininess;
	glm::vec3 m_ambient;
	glm::vec3 m_diffuse;
	glm::vec3 m_specular;
};
