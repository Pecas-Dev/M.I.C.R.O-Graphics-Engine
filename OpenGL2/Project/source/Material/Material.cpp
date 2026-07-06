#include <Material/Material.h>
#include <Utility/Utility.h>

#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstring>
#include <filesystem>
#include <iostream>
#include <fstream>


Material::Material()
{
	m_shininess = 0.0f;
	m_roughness = 0.65f;
	m_metallic = 0.0f;
	m_ambient = glm::vec3(0.0f);
	m_diffuse = glm::vec3(0.0f);
	m_specular = glm::vec3(0.0f);
	m_isTextured = false;
	m_hasNormalMap = false;
	m_hasRoughnessMap = false;
	m_hasMetallicMap = false;
}

const std::string& Material::GetName() const
{
	return m_name;
}

bool Material::IsTextured() const
{
	return m_isTextured;
}

const Texture& Material::GetNormalMap() const
{
	return m_normalMap;
}

const Texture& Material::GetDiffuseMap() const
{
	return m_diffuseMap;
}

const Texture& Material::GetSpecularMap() const
{
	return m_specularMap;
}

const Texture& Material::GetAmbientMap() const
{
	return m_ambientMap;
}

void Material::SetName(const std::string& name)
{
	m_name = name;
}

void Material::SetShininess(GLfloat shininess)
{
	m_shininess = shininess;
}

void Material::SetRoughness(GLfloat roughness)
{
	m_roughness = roughness;
}

void Material::SetMetallic(GLfloat metallic)
{
	m_metallic = metallic;
}

void Material::SetAmbient(const glm::vec3& ambient)
{
	m_ambient.r = ambient.r;
	m_ambient.g = ambient.g;
	m_ambient.b = ambient.b;
}

void Material::SetDiffuse(const glm::vec3& diffuse)
{
	m_diffuse.r = diffuse.r;
	m_diffuse.g = diffuse.g;
	m_diffuse.b = diffuse.b;
}

void Material::SetSpecular(const glm::vec3& specular)
{
	m_specular.r = specular.r;
	m_specular.g = specular.g;
	m_specular.b = specular.b;
}

namespace
{
	bool FileExists(const std::string& path)
	{
		std::ifstream file(path);
		return file.good();
	}

	std::string RestOfLine(const std::string& line, size_t keywordLength)
	{
		size_t position = line.find_first_not_of(" \t", keywordLength);
		return (position == std::string::npos) ? std::string() : line.substr(position);
	}

	std::string ResolvePath(const std::string& filename, const std::string& directory, const std::string& assetDirectory)
	{
		if (!directory.empty() && FileExists(directory + filename)) { return directory + filename; }
		if (FileExists(assetDirectory + filename)) { return assetDirectory + filename; }
		if (FileExists(filename)) { return filename; }

		return std::string();
	}

	std::string ToLower(const std::string& text)
	{
		std::string result = text;
		std::transform(result.begin(), result.end(), result.begin(),
			[](unsigned char character) { return static_cast<char>(std::tolower(character)); });
		return result;
	}

	bool HasImageExtension(const std::string& lowercaseName)
	{
		const char* extensions[] = { ".png", ".jpg", ".jpeg", ".bmp", ".tga" };

		for (const char* extension : extensions)
		{
			size_t length = std::strlen(extension);

			if (lowercaseName.size() >= length && lowercaseName.compare(lowercaseName.size() - length, length, extension) == 0)
			{
				return true;
			}
		}

		return false;
	}

	std::string FindTextureByKeywords(const std::string& directory, const char* const* keywords, size_t totalKeywords)
	{
		const std::string searchFolders[] = { directory, directory + "textures/" };

		for (const auto& folder : searchFolders)
		{
			std::error_code errorCode;

			if (folder.empty() || !std::filesystem::is_directory(folder, errorCode)) { continue; }

			for (const auto& entry : std::filesystem::directory_iterator(folder, errorCode))
			{
				if (!entry.is_regular_file(errorCode)) { continue; }

				std::string name = ToLower(entry.path().filename().string());

				if (!HasImageExtension(name)) { continue; }

				for (size_t i = 0; i < totalKeywords; i++)
				{
					if (name.find(keywords[i]) != std::string::npos)
					{
						return entry.path().string();
					}
				}
			}
		}

		return std::string();
	}

	std::string ResolveTextureFile(const std::string& filename, const std::string& modelDirectory)
	{
		std::string texturePath = ResolvePath(filename, modelDirectory, "Assets/Textures/");

		if (!texturePath.empty()) { return texturePath; }

		size_t lastSlash = filename.find_last_of("/\\");
		std::string baseName = (lastSlash != std::string::npos) ? filename.substr(lastSlash + 1) : filename;

		std::string stem = baseName;
		size_t lastDot = stem.find_last_of('.');

		if (lastDot != std::string::npos)
		{
			stem = stem.substr(0, lastDot);
		}

		const std::string extensions[] = { "", ".png", ".jpg", ".jpeg", ".bmp", ".tga" };

		for (const auto& extension : extensions)
		{
			std::string candidate = extension.empty() ? baseName : (stem + extension);

			texturePath = ResolvePath(candidate, modelDirectory, "Assets/Textures/");

			if (texturePath.empty())
			{
				texturePath = ResolvePath("textures/" + candidate, modelDirectory, "Assets/Textures/");
			}

			if (!texturePath.empty()) { break; }
		}

		return texturePath;
	}
}

bool Material::LoadDiffuseMap(const std::string& filename, const std::string& modelDirectory)
{
	std::string texturePath = ResolveTextureFile(filename, modelDirectory);

	if (texturePath.empty())
	{
		Utility::AddMessage("Texture not found: " + filename);
		return false;
	}

	if (m_diffuseMap.Load(texturePath))
	{
		m_isTextured = true;
		return true;
	}

	return false;
}

bool Material::LoadNormalMap(const std::string& filename, const std::string& modelDirectory)
{
	std::string texturePath = ResolveTextureFile(filename, modelDirectory);

	if (!texturePath.empty() && m_normalMap.Load(texturePath))
	{
		m_hasNormalMap = true;
		return true;
	}

	return false;
}

bool Material::LoadRoughnessMap(const std::string& filename, const std::string& modelDirectory)
{
	std::string texturePath = ResolveTextureFile(filename, modelDirectory);

	if (!texturePath.empty() && m_roughnessMap.Load(texturePath))
	{
		m_hasRoughnessMap = true;
		return true;
	}

	return false;
}

bool Material::LoadMetallicMap(const std::string& filename, const std::string& modelDirectory)
{
	std::string texturePath = ResolveTextureFile(filename, modelDirectory);

	if (!texturePath.empty() && m_metallicMap.Load(texturePath))
	{
		m_hasMetallicMap = true;
		return true;
	}

	return false;
}

void Material::ScanForMaps(const std::string& modelDirectory)
{
	const char* baseColorKeywords[] = { "basecolor", "base_color", "albedo", "diffuse", "_col" };
	const char* normalKeywords[] = { "normal", "_nrm", "_nor" };
	const char* roughnessKeywords[] = { "roughness", "_rgh" };
	const char* metallicKeywords[] = { "metallic", "metalness" };

	if (!m_isTextured)
	{
		std::string path = FindTextureByKeywords(modelDirectory, baseColorKeywords, 5);

		if (!path.empty() && m_diffuseMap.Load(path))
		{
			Utility::AddMessage("Found base color texture: " + path);
			m_isTextured = true;
		}
	}

	if (!m_hasNormalMap)
	{
		std::string path = FindTextureByKeywords(modelDirectory, normalKeywords, 3);
		if (!path.empty() && m_normalMap.Load(path)) { m_hasNormalMap = true; }
	}

	if (!m_hasRoughnessMap)
	{
		std::string path = FindTextureByKeywords(modelDirectory, roughnessKeywords, 2);
		if (!path.empty() && m_roughnessMap.Load(path)) { m_hasRoughnessMap = true; }
	}

	if (!m_hasMetallicMap)
	{
		std::string path = FindTextureByKeywords(modelDirectory, metallicKeywords, 2);
		if (!path.empty() && m_metallicMap.Load(path)) { m_hasMetallicMap = true; }
	}
}

bool Material::LoadDiffuseMapFromMemory(const void* data, int size)
{
	if (m_diffuseMap.LoadFromMemory(data, size))
	{
		m_isTextured = true;
		return true;
	}

	return false;
}

bool Material::LoadNormalMapFromMemory(const void* data, int size)
{
	if (m_normalMap.LoadFromMemory(data, size))
	{
		m_hasNormalMap = true;
		return true;
	}

	return false;
}

bool Material::LoadRoughnessMapFromMemory(const void* data, int size)
{
	if (m_roughnessMap.LoadFromMemory(data, size))
	{
		m_hasRoughnessMap = true;
		return true;
	}

	return false;
}

bool Material::LoadMetallicMapFromMemory(const void* data, int size)
{
	if (m_metallicMap.LoadFromMemory(data, size))
	{
		m_hasMetallicMap = true;
		return true;
	}

	return false;
}

bool Material::Load(const std::string& filename, std::vector<Material>& materials, const std::string& modelDirectory)
{
	std::string path = ResolvePath(filename, modelDirectory, "Assets/Materials/");

	if (path.empty())
	{
		Utility::AddMessage("Material file not found: " + filename);
		return false;
	}

	std::fstream file(path, std::ios_base::in);

	if (!file)
	{
		Utility::AddMessage("Error loading material file: " + path);
		return false;
	}

	std::string materialDirectory;
	size_t lastSlash = path.find_last_of("/\\");

	if (lastSlash != std::string::npos)
	{
		materialDirectory = path.substr(0, lastSlash + 1);
	}

	auto LoadMap = [&](Texture& map, const std::string& mapFilename)
	{
		std::string texturePath = ResolvePath(mapFilename, materialDirectory, "Assets/Textures/");

		if (texturePath.empty() && materialDirectory != modelDirectory)
		{
			texturePath = ResolvePath(mapFilename, modelDirectory, "Assets/Textures/");
		}

		if (texturePath.empty())
		{
			Utility::AddMessage("Texture not found: " + mapFilename);
			return false;
		}

		return map.Load(texturePath);
	};

	std::string line;
	std::vector<std::string> subStrings;

	while (std::getline(file, line))
	{
		if (!line.empty() && line.back() == '\r') { line.pop_back(); }
		if (line.empty() || line[0] == '#') { continue; }

		subStrings.clear();
		Utility::ParseString(line, subStrings, ' ');

		if (subStrings.empty()) { continue; }

		const std::string& keyword = subStrings[0];

		try
		{
			if (keyword == "newmtl" && subStrings.size() >= 2)
			{
				materials.push_back(Material());
				materials.back().m_name = RestOfLine(line, keyword.length());

				materials.back().SetAmbient(glm::vec3(1.0f));
				materials.back().SetDiffuse(glm::vec3(1.0f));
				continue;
			}

			if (materials.empty()) { continue; }

			if (keyword == "Ka" && subStrings.size() >= 4)
			{
				materials.back().SetAmbient(glm::vec3(std::stof(subStrings[1]), std::stof(subStrings[2]), std::stof(subStrings[3])));
			}

			else if (keyword == "Kd" && subStrings.size() >= 4)
			{
				materials.back().SetDiffuse(glm::vec3(std::stof(subStrings[1]), std::stof(subStrings[2]), std::stof(subStrings[3])));
			}

			else if (keyword == "Ks" && subStrings.size() >= 4)
			{
				materials.back().SetSpecular(glm::vec3(std::stof(subStrings[1]), std::stof(subStrings[2]), std::stof(subStrings[3])));
			}

			else if (keyword == "Ns" && subStrings.size() >= 2)
			{
				float shininess = std::stof(subStrings[1]);
				materials.back().SetShininess(shininess);

				materials.back().SetRoughness(glm::clamp(std::sqrt(2.0f / (shininess + 2.0f)), 0.05f, 1.0f));
			}

			else if (keyword == "map_Ka" && subStrings.size() >= 2)
			{
				LoadMap(materials.back().m_ambientMap, RestOfLine(line, keyword.length()));
			}

			else if (keyword == "map_Kd" && subStrings.size() >= 2)
			{
				if (LoadMap(materials.back().m_diffuseMap, RestOfLine(line, keyword.length())))
				{
					materials.back().m_isTextured = true;
				}
			}

			else if (keyword == "map_Ks" && subStrings.size() >= 2)
			{
				LoadMap(materials.back().m_specularMap, RestOfLine(line, keyword.length()));
			}

			else if ((keyword == "map_Ns" || keyword == "bump" || keyword == "map_bump") && subStrings.size() >= 2)
			{
				if (LoadMap(materials.back().m_normalMap, RestOfLine(line, keyword.length())))
				{
					materials.back().m_hasNormalMap = true;
				}
			}
		}
		catch (const std::exception&)
		{
			Utility::AddMessage("Skipped invalid material line: " + line);
		}
	}

	file.close();

	return true;
}

void Material::SendToShader(const Shader& shader)
{
	shader.SendUniformData("material.shininess", m_shininess);
	shader.SendUniformData("material.roughness", m_roughness);
	shader.SendUniformData("material.metallic", m_metallic);
	shader.SendUniformData("material.ambient", m_ambient.r, m_ambient.g, m_ambient.b);
	shader.SendUniformData("material.diffuse", m_diffuse.r, m_diffuse.g, m_diffuse.b);
	shader.SendUniformData("material.specular", m_specular.r, m_specular.g, m_specular.b);
}

void Material::BindMaps(const Shader& shader, bool texturingEnabled) const
{
	bool useAlbedo = m_isTextured && texturingEnabled;
	bool useNormal = m_hasNormalMap && texturingEnabled;
	bool useRoughness = m_hasRoughnessMap && texturingEnabled;
	bool useMetallic = m_hasMetallicMap && texturingEnabled;

	shader.SendUniformData("isTextured", useAlbedo);
	shader.SendUniformData("hasNormalMap", useNormal);
	shader.SendUniformData("hasRoughnessMap", useRoughness);
	shader.SendUniformData("hasMetallicMap", useMetallic);

	shader.SendUniformData("textureImage", 0);
	shader.SendUniformData("normalMap", 1);
	shader.SendUniformData("roughnessMap", 2);
	shader.SendUniformData("metallicMap", 3);

	if (useAlbedo) { m_diffuseMap.Bind(0u); }
	if (useNormal) { m_normalMap.Bind(1u); }
	if (useRoughness) { m_roughnessMap.Bind(2u); }
	if (useMetallic) { m_metallicMap.Bind(3u); }
}
