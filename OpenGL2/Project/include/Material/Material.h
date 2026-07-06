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

	void SetName(const std::string& name);
	void SetShininess(GLfloat shininess);
	void SetRoughness(GLfloat roughness);
	void SetMetallic(GLfloat metallic);
	void SetAmbient(const glm::vec3& ambient);
	void SetDiffuse(const glm::vec3& diffuse);
	void SetSpecular(const glm::vec3& specular);

	bool LoadDiffuseMap(const std::string& filename, const std::string& modelDirectory);
	bool LoadNormalMap(const std::string& filename, const std::string& modelDirectory);
	bool LoadRoughnessMap(const std::string& filename, const std::string& modelDirectory);
	bool LoadMetallicMap(const std::string& filename, const std::string& modelDirectory);
	bool LoadDiffuseMapFromMemory(const void* data, int size);
	bool LoadNormalMapFromMemory(const void* data, int size);
	bool LoadRoughnessMapFromMemory(const void* data, int size);
	bool LoadMetallicMapFromMemory(const void* data, int size);

	void ScanForMaps(const std::string& modelDirectory);

	bool Load(const std::string& filename, std::vector<Material>& materials, const std::string& modelDirectory = "");

	void SendToShader(const Shader& shader);

	void BindMaps(const Shader& shader, bool texturingEnabled) const;

private:
	bool m_isTextured;
	std::string m_name;

	Texture m_normalMap;
	Texture m_diffuseMap;
	Texture m_specularMap;
	Texture m_ambientMap;
	Texture m_roughnessMap;
	Texture m_metallicMap;

	bool m_hasNormalMap;
	bool m_hasRoughnessMap;
	bool m_hasMetallicMap;

	GLfloat m_shininess;
	GLfloat m_roughness;
	GLfloat m_metallic;
	glm::vec3 m_ambient;
	glm::vec3 m_diffuse;
	glm::vec3 m_specular;
};
