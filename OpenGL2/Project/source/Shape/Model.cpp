#include <InputSystem/Input.h>
#include <Utility/Utility.h>
#include <Shape/Model.h>

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#include <cctype>
#include <cmath>
#include <fstream>
#include <iostream>
#include <limits>
#include <unordered_map>


namespace
{
	bool ParseVertexGroup(const std::string& token, VertexGroup& group, int totalVertices, int totalTextureCoordinates, int totalNormals)
	{
		int indices[3] = { 0, 0, 0 };
		bool isPresent[3] = { false, false, false };

		size_t part = 0;
		size_t start = 0;

		while (part < 3)
		{
			size_t end = token.find('/', start);
			std::string number = token.substr(start, (end == std::string::npos) ? std::string::npos : end - start);

			if (!number.empty())
			{
				try
				{
					indices[part] = std::stoi(number);
				}
				catch (...)
				{
					return false;
				}

				isPresent[part] = true;
			}

			if (end == std::string::npos) { break; }

			start = end + 1;
			part++;
		}

		auto Resolve = [](int index, int total)
		{
			return (index > 0) ? index - 1 : total + index;
		};

		if (!isPresent[0]) { return false; }

		group.v = Resolve(indices[0], totalVertices);
		group.t = isPresent[1] ? Resolve(indices[1], totalTextureCoordinates) : -1;
		group.n = isPresent[2] ? Resolve(indices[2], totalNormals) : -1;

		if (group.v < 0 || group.v >= totalVertices) { return false; }

		if (group.t < 0 || group.t >= totalTextureCoordinates) { group.t = -1; }
		if (group.n < 0 || group.n >= totalNormals) { group.n = -1; }

		return true;
	}

	std::string RestOfLine(const std::string& line, size_t keywordLength)
	{
		size_t position = line.find_first_not_of(" \t", keywordLength);
		return (position == std::string::npos) ? std::string() : line.substr(position);
	}
}


Model::Model(Grid* parentGrid) : Object(parentGrid)
{
}

bool Model::Load(const std::string& filename)
{
	m_sourceFile = filename;

	std::string directory;
	size_t lastSlash = filename.find_last_of("/\\");

	if (lastSlash != std::string::npos)
	{
		directory = filename.substr(0, lastSlash + 1);
	}

	std::string extension;
	size_t lastDot = filename.find_last_of('.');

	if (lastDot != std::string::npos)
	{
		extension = filename.substr(lastDot + 1);
	}

	for (auto& character : extension)
	{
		character = static_cast<char>(std::tolower(static_cast<unsigned char>(character)));
	}

	if (extension == "obj")
	{
		return LoadOBJ(filename, directory);
	}

	return LoadWithAssimp(filename, directory);
}

bool Model::LoadOBJ(const std::string& filename, const std::string& directory)
{
	std::fstream file(filename, std::ios_base::in);

	if (!file)
	{
		Utility::AddMessage("Failed to open model file: " + filename);
		return false;
	}

	Mesh rawMesh;
	std::string lastMaterialName;

	std::string line;
	std::string lastName;
	std::vector<Face> faces;

	std::vector<std::string> subStrings;
	subStrings.reserve(16);

	int lineNumber = 0;
	int totalSkippedFaces = 0;
	int totalInvalidLines = 0;

	auto FlushMesh = [&]()
	{
		if (faces.empty()) { return; }

		Mesh mesh;
		mesh.name = lastName;
		mesh.materialName = lastMaterialName;
		SortVertexData(mesh, rawMesh, faces);
		m_meshes.push_back(mesh);

		faces.clear();
	};

	while (std::getline(file, line))
	{
		lineNumber++;

		if (!line.empty() && line.back() == '\r') { line.pop_back(); }
		if (line.empty() || line[0] == '#') { continue; }

		subStrings.clear();
		Utility::ParseString(line, subStrings, ' ');

		if (subStrings.empty()) { continue; }

		const std::string& keyword = subStrings[0];

		try
		{
			if (keyword == "v" && subStrings.size() >= 4)
			{
				rawMesh.vertices.push_back(glm::vec3(std::stof(subStrings[1]), std::stof(subStrings[2]), std::stof(subStrings[3])));
			}

			else if (keyword == "vn" && subStrings.size() >= 4)
			{
				rawMesh.normals.push_back(glm::vec3(std::stof(subStrings[1]), std::stof(subStrings[2]), std::stof(subStrings[3])));
			}

			else if (keyword == "vt" && subStrings.size() >= 3)
			{
				rawMesh.textureCoordinates.push_back(glm::vec2(std::stof(subStrings[1]), std::stof(subStrings[2])));
			}

			else if (keyword == "f" && subStrings.size() >= 4)
			{
				std::vector<VertexGroup> vertexGroups;
				vertexGroups.reserve(subStrings.size() - 1);

				bool isFaceValid = true;

				for (size_t i = 1; i < subStrings.size(); i++)
				{
					VertexGroup vertexGroup;

					if (!ParseVertexGroup(subStrings[i], vertexGroup,
						static_cast<int>(rawMesh.vertices.size()),
						static_cast<int>(rawMesh.textureCoordinates.size()),
						static_cast<int>(rawMesh.normals.size())))
					{
						isFaceValid = false;
						break;
					}

					vertexGroups.push_back(vertexGroup);
				}

				if (isFaceValid)
				{
					for (size_t i = 1; i + 1 < vertexGroups.size(); i++)
					{
						Face face;
						face.push_back(vertexGroups[0]);
						face.push_back(vertexGroups[i]);
						face.push_back(vertexGroups[i + 1]);
						faces.push_back(face);
					}
				}
				else
				{
					totalSkippedFaces++;
				}
			}

			else if (keyword == "usemtl" && subStrings.size() >= 2)
			{
				FlushMesh();
				lastMaterialName = RestOfLine(line, keyword.length());
			}

			else if (keyword == "mtllib" && subStrings.size() >= 2)
			{
				Material material;
				material.Load(RestOfLine(line, keyword.length()), m_materials, directory);
			}

			else if (keyword == "g" || keyword == "o")
			{
				FlushMesh();
				lastName = (subStrings.size() >= 2) ? subStrings[1] : std::string();
			}
		}
		catch (const std::exception&)
		{
			totalInvalidLines++;
		}
	}

	file.close();

	FlushMesh();

	if (totalSkippedFaces > 0)
	{
		Utility::AddMessage("Skipped " + std::to_string(totalSkippedFaces) + " malformed face(s) in: " + filename);
	}

	if (totalInvalidLines > 0)
	{
		Utility::AddMessage("Skipped " + std::to_string(totalInvalidLines) + " invalid line(s) in: " + filename);
	}

	if (m_meshes.empty())
	{
		Utility::AddMessage("No renderable geometry found in: " + filename);
		return false;
	}

	FillBuffers();

	return true;
}

bool Model::LoadWithAssimp(const std::string& filename, const std::string& directory)
{
	Assimp::Importer importer;

	const aiScene* scene = importer.ReadFile(filename,
		aiProcess_Triangulate |
		aiProcess_JoinIdenticalVertices |
		aiProcess_GenSmoothNormals |
		aiProcess_CalcTangentSpace |
		aiProcess_SortByPType |
		aiProcess_PreTransformVertices);

	if (!scene || !scene->mRootNode || (scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE))
	{
		Utility::AddMessage("Failed to load model: " + std::string(importer.GetErrorString()));
		return false;
	}

	std::vector<std::string> materialNames;
	materialNames.reserve(scene->mNumMaterials);

	for (unsigned int i = 0; i < scene->mNumMaterials; i++)
	{
		const aiMaterial* sceneMaterial = scene->mMaterials[i];

		aiString name;
		sceneMaterial->Get(AI_MATKEY_NAME, name);

		Material material;
		material.SetName(std::string(name.C_Str()) + "#" + std::to_string(i));

		aiColor3D diffuse(0.8f, 0.8f, 0.8f);
		sceneMaterial->Get(AI_MATKEY_COLOR_DIFFUSE, diffuse);
		material.SetDiffuse(glm::vec3(diffuse.r, diffuse.g, diffuse.b));

		aiColor3D ambient(0.0f, 0.0f, 0.0f);

		if (sceneMaterial->Get(AI_MATKEY_COLOR_AMBIENT, ambient) == AI_SUCCESS && (ambient.r + ambient.g + ambient.b) > 0.0f)
		{
			material.SetAmbient(glm::vec3(ambient.r, ambient.g, ambient.b));
		}
		else
		{
			material.SetAmbient(glm::vec3(diffuse.r, diffuse.g, diffuse.b));
		}

		aiColor3D specular(0.0f, 0.0f, 0.0f);
		sceneMaterial->Get(AI_MATKEY_COLOR_SPECULAR, specular);
		material.SetSpecular(glm::vec3(specular.r, specular.g, specular.b));

		float shininess = 0.0f;
		sceneMaterial->Get(AI_MATKEY_SHININESS, shininess);
		material.SetShininess((shininess > 0.0f) ? shininess : 32.0f);

		float roughness = 0.0f;
		float metallic = 0.0f;

		if (sceneMaterial->Get(AI_MATKEY_ROUGHNESS_FACTOR, roughness) == AI_SUCCESS && roughness > 0.0f)
		{
			material.SetRoughness(glm::clamp(roughness, 0.05f, 1.0f));
		}
		else if (shininess > 0.0f)
		{
			material.SetRoughness(glm::clamp(std::sqrt(2.0f / (shininess + 2.0f)), 0.05f, 1.0f));
		}

		if (sceneMaterial->Get(AI_MATKEY_METALLIC_FACTOR, metallic) == AI_SUCCESS)
		{
			material.SetMetallic(glm::clamp(metallic, 0.0f, 1.0f));
		}

		aiString texturePath;

		if (sceneMaterial->GetTexture(aiTextureType_DIFFUSE, 0, &texturePath) == AI_SUCCESS ||
			sceneMaterial->GetTexture(aiTextureType_BASE_COLOR, 0, &texturePath) == AI_SUCCESS)
		{
			const aiTexture* embeddedTexture = scene->GetEmbeddedTexture(texturePath.C_Str());

			if (embeddedTexture)
			{
				if (embeddedTexture->mHeight == 0)
				{
					material.LoadDiffuseMapFromMemory(embeddedTexture->pcData, static_cast<int>(embeddedTexture->mWidth));
				}
				else
				{
					Utility::AddMessage("Uncompressed embedded textures are not supported yet.");
				}
			}
			else
			{
				material.LoadDiffuseMap(texturePath.C_Str(), directory);
			}
		}

		auto LoadEmbeddedOrFile = [&](const aiString& path, auto fromMemory, auto fromFile)
		{
			const aiTexture* embeddedTexture = scene->GetEmbeddedTexture(path.C_Str());

			if (embeddedTexture)
			{
				if (embeddedTexture->mHeight == 0)
				{
					(material.*fromMemory)(embeddedTexture->pcData, static_cast<int>(embeddedTexture->mWidth));
				}
			}
			else
			{
				(material.*fromFile)(path.C_Str(), directory);
			}
		};

		if (sceneMaterial->GetTexture(aiTextureType_NORMALS, 0, &texturePath) == AI_SUCCESS ||
			sceneMaterial->GetTexture(aiTextureType_HEIGHT, 0, &texturePath) == AI_SUCCESS)
		{
			LoadEmbeddedOrFile(texturePath, &Material::LoadNormalMapFromMemory, &Material::LoadNormalMap);
		}

		if (sceneMaterial->GetTexture(aiTextureType_DIFFUSE_ROUGHNESS, 0, &texturePath) == AI_SUCCESS ||
			sceneMaterial->GetTexture(aiTextureType_SHININESS, 0, &texturePath) == AI_SUCCESS)
		{
			LoadEmbeddedOrFile(texturePath, &Material::LoadRoughnessMapFromMemory, &Material::LoadRoughnessMap);
		}

		if (sceneMaterial->GetTexture(aiTextureType_METALNESS, 0, &texturePath) == AI_SUCCESS)
		{
			LoadEmbeddedOrFile(texturePath, &Material::LoadMetallicMapFromMemory, &Material::LoadMetallicMap);
		}

		if (scene->mNumMaterials == 1)
		{
			material.ScanForMaps(directory);
		}

		m_materials.push_back(material);
		materialNames.push_back(material.GetName());
	}

	for (unsigned int i = 0; i < scene->mNumMeshes; i++)
	{
		const aiMesh* sceneMesh = scene->mMeshes[i];

		if (!sceneMesh->HasPositions() || sceneMesh->mNumFaces == 0 || (sceneMesh->mPrimitiveTypes & aiPrimitiveType_TRIANGLE) == 0)
		{
			continue;
		}

		Mesh mesh;
		mesh.name = sceneMesh->mName.C_Str();
		mesh.materialName = (sceneMesh->mMaterialIndex < materialNames.size()) ? materialNames[sceneMesh->mMaterialIndex] : std::string();

		mesh.vertices.reserve(sceneMesh->mNumVertices);
		mesh.normals.reserve(sceneMesh->mNumVertices);
		mesh.textureCoordinates.reserve(sceneMesh->mNumVertices);
		mesh.indices.reserve(sceneMesh->mNumFaces * 3);

		const bool hasTextureCoordinates = (sceneMesh->mTextureCoords[0] != nullptr);
		const bool hasTangents = (sceneMesh->mTangents != nullptr);

		for (unsigned int v = 0; v < sceneMesh->mNumVertices; v++)
		{
			mesh.vertices.push_back(glm::vec3(sceneMesh->mVertices[v].x, sceneMesh->mVertices[v].y, sceneMesh->mVertices[v].z));
			mesh.normals.push_back(glm::vec3(sceneMesh->mNormals[v].x, sceneMesh->mNormals[v].y, sceneMesh->mNormals[v].z));

			if (hasTangents)
			{
				mesh.tangents.push_back(glm::vec3(sceneMesh->mTangents[v].x, sceneMesh->mTangents[v].y, sceneMesh->mTangents[v].z));
			}

			if (hasTextureCoordinates)
			{
				mesh.textureCoordinates.push_back(glm::vec2(sceneMesh->mTextureCoords[0][v].x, sceneMesh->mTextureCoords[0][v].y));
			}
			else
			{
				mesh.textureCoordinates.push_back(glm::vec2(0.0f));
			}
		}

		for (unsigned int f = 0; f < sceneMesh->mNumFaces; f++)
		{
			const aiFace& face = sceneMesh->mFaces[f];

			if (face.mNumIndices != 3) { continue; }

			for (unsigned int index = 0; index < face.mNumIndices; index++)
			{
				mesh.indices.push_back(face.mIndices[index]);
			}
		}

		m_meshes.push_back(mesh);
	}

	if (m_meshes.empty())
	{
		Utility::AddMessage("No renderable geometry found in: " + filename);
		return false;
	}

	FillBuffers();

	return true;
}

void Model::Render(const Shader& shader)
{
	Object::Render(shader);

	shader.SendUniformData("isTextured", m_isTextured);
	//shader.SendUniformData("isLit", false);

	int count = 0;

	for (auto& buffer : m_buffers)
	{
		buffer.LinkEBO();

		buffer.LinkVBO(shader, "vertexIn", Buffer::VBOType::VBOT_VertexBuffer, Buffer::ComponentType::CT_XYZ, Buffer::DataType::DT_Float);
		buffer.LinkVBO(shader, "colorIn", Buffer::VBOType::VBOT_ColorBuffer, Buffer::ComponentType::CT_RGBA, Buffer::DataType::DT_Float);
		buffer.LinkVBO(shader, "textureIn", Buffer::VBOType::VBOT_TextureBuffer, Buffer::ComponentType::CT_UV, Buffer::DataType::DT_Float);
		buffer.LinkVBO(shader, "normalIn", Buffer::VBOType::VBOT_NormalBuffer, Buffer::ComponentType::CT_XYZ, Buffer::DataType::DT_Float);
		buffer.LinkVBO(shader, "tangentIn", Buffer::VBOType::VBOT_TangentBuffer, Buffer::ComponentType::CT_XYZ, Buffer::DataType::DT_Float);

		bool materialFound = false;

		for (auto& mat : m_materials)
		{
			if (mat.GetName() == m_meshes[count].materialName)
			{
				mat.SendToShader(shader);
				mat.BindMaps(shader, m_isTextured);

				materialFound = true;
				break;
			}
		}

		if (!materialFound)
		{
			shader.SendUniformData("isTextured", false);
			shader.SendUniformData("hasNormalMap", false);
			shader.SendUniformData("hasRoughnessMap", false);
			shader.SendUniformData("hasMetallicMap", false);
			shader.SendUniformData("material.shininess", 32.0f);
			shader.SendUniformData("material.roughness", 0.65f);
			shader.SendUniformData("material.metallic", 0.0f);
			shader.SendUniformData("material.ambient", 1.0f, 1.0f, 1.0f);
			shader.SendUniformData("material.diffuse", 0.8f, 0.8f, 0.8f);
			shader.SendUniformData("material.specular", 0.4f, 0.4f, 0.4f);
		}

		if (!m_presetName.empty())
		{
			shader.SendUniformData("isTextured", false);
			shader.SendUniformData("hasRoughnessMap", false);
			shader.SendUniformData("hasMetallicMap", false);
			shader.SendUniformData("material.roughness", m_presetRoughness);
			shader.SendUniformData("material.metallic", m_presetMetallic);
			shader.SendUniformData("material.diffuse", m_presetTint.r, m_presetTint.g, m_presetTint.b);
			shader.SendUniformData("material.ambient", m_presetTint.r, m_presetTint.g, m_presetTint.b);
			shader.SendUniformData("material.specular", 1.0f, 1.0f, 1.0f);
		}

		count++;

		buffer.Render(Buffer::DrawType::DwT_Triangles);
	}
}

bool Model::HasGeometry() const
{
	return !m_meshes.empty();
}

bool Model::HasTexturedMaterial() const
{
	for (const auto& material : m_materials)
	{
		if (material.IsTextured()) { return true; }
	}

	return false;
}

int Model::GetMeshCount() const
{
	return static_cast<int>(m_meshes.size());
}

int Model::GetVertexCount() const
{
	size_t count = 0;

	for (const auto& mesh : m_meshes)
	{
		count += mesh.vertices.size();
	}

	return static_cast<int>(count);
}

int Model::GetTriangleCount() const
{
	size_t count = 0;

	for (const auto& mesh : m_meshes)
	{
		count += mesh.indices.size() / 3;
	}

	return static_cast<int>(count);
}

const std::string& Model::GetSourceFile() const
{
	return m_sourceFile;
}

std::string Model::GetDisplayName() const
{
	size_t lastSlash = m_sourceFile.find_last_of("/\\");
	return (lastSlash == std::string::npos) ? m_sourceFile : m_sourceFile.substr(lastSlash + 1);
}

bool& Model::Visible()
{
	return m_visible;
}

const glm::vec3& Model::GetBoundsMin() const
{
	return m_boundsMin;
}

const glm::vec3& Model::GetBoundsMax() const
{
	return m_boundsMax;
}

void Model::ApplyMaterialPreset(const std::string& name, const glm::vec3& tint, float roughness, float metallic)
{
	m_presetName = name;
	m_presetTint = tint;
	m_presetRoughness = roughness;
	m_presetMetallic = metallic;
}

void Model::ClearMaterialPreset()
{
	m_presetName.clear();
}

const std::string& Model::GetMaterialPresetName() const
{
	return m_presetName;
}

void Model::RenderDepth(const Shader& shader)
{
	Object::Render(shader);

	for (auto& buffer : m_buffers)
	{
		buffer.LinkEBO();
		buffer.LinkVBO(shader, "vertexIn", Buffer::VBOType::VBOT_VertexBuffer, Buffer::ComponentType::CT_XYZ, Buffer::DataType::DT_Float);
		buffer.Render(Buffer::DrawType::DwT_Triangles);
	}
}

void Model::SetColor(const glm::vec4& color)
{
	auto count = 0;

	for (auto& mesh : m_meshes)
	{
		for (auto& col : mesh.colors)
		{
			glm::vec4 newColor(color.r, color.g, color.b, color.a);
			col = newColor;
		}

		m_buffers[count++].FillVBO(Buffer::VBOType::VBOT_ColorBuffer, &mesh.colors[0].x, mesh.colors.size() * sizeof(glm::vec4), Buffer::FillType::FT_Multiple);
	}

	m_color = color;
}

void Model::ComputeTangents(Mesh& mesh)
{
	mesh.tangents.assign(mesh.vertices.size(), glm::vec3(0.0f));

	for (size_t i = 0; i + 2 < mesh.indices.size(); i += 3)
	{
		GLuint i0 = mesh.indices[i];
		GLuint i1 = mesh.indices[i + 1];
		GLuint i2 = mesh.indices[i + 2];

		glm::vec3 edge1 = mesh.vertices[i1] - mesh.vertices[i0];
		glm::vec3 edge2 = mesh.vertices[i2] - mesh.vertices[i0];
		glm::vec2 deltaUV1 = mesh.textureCoordinates[i1] - mesh.textureCoordinates[i0];
		glm::vec2 deltaUV2 = mesh.textureCoordinates[i2] - mesh.textureCoordinates[i0];

		float determinant = deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y;

		glm::vec3 tangent = (std::fabs(determinant) > 1e-8f)
			? (edge1 * deltaUV2.y - edge2 * deltaUV1.y) / determinant
			: edge1; 

		mesh.tangents[i0] += tangent;
		mesh.tangents[i1] += tangent;
		mesh.tangents[i2] += tangent;
	}

	for (size_t i = 0; i < mesh.tangents.size(); i++)
	{
		if (glm::dot(mesh.tangents[i], mesh.tangents[i]) > 1e-12f)
		{
			mesh.tangents[i] = glm::normalize(mesh.tangents[i]);
		}
		else
		{
			const glm::vec3& normal = mesh.normals[i];
			glm::vec3 helper = (std::fabs(normal.y) < 0.99f) ? glm::vec3(0.0f, 1.0f, 0.0f) : glm::vec3(1.0f, 0.0f, 0.0f);
			mesh.tangents[i] = glm::normalize(glm::cross(helper, normal));
		}
	}
}

void Model::FillBuffers()
{
	NormalizeSize();

	for (auto& mesh : m_meshes)
	{
		if (mesh.tangents.size() != mesh.vertices.size())
		{
			ComputeTangents(mesh);
		}

		Buffer buffer;
		buffer.CreateBuffer(static_cast<GLuint>(mesh.indices.size()), true);

		buffer.FillEBO(&mesh.indices[0], mesh.indices.size() * sizeof(GLuint), Buffer::FillType::FT_Single);
		buffer.FillVBO(Buffer::VBOType::VBOT_VertexBuffer, &mesh.vertices[0].x, mesh.vertices.size() * sizeof(glm::vec3), Buffer::FillType::FT_Single);
		buffer.FillVBO(Buffer::VBOType::VBOT_TextureBuffer, &mesh.textureCoordinates[0].x, mesh.textureCoordinates.size() * sizeof(glm::vec2), Buffer::FillType::FT_Single);
		buffer.FillVBO(Buffer::VBOType::VBOT_NormalBuffer, &mesh.normals[0].x, mesh.normals.size() * sizeof(glm::vec3), Buffer::FillType::FT_Single);
		buffer.FillVBO(Buffer::VBOType::VBOT_TangentBuffer, &mesh.tangents[0].x, mesh.tangents.size() * sizeof(glm::vec3), Buffer::FillType::FT_Single);

		for (const auto& vertex : mesh.vertices)
		{
			glm::vec4 color(1.0f, 1.0f, 1.0f, 1.0f);
			mesh.colors.push_back(color);
		}

		buffer.FillVBO(Buffer::VBOType::VBOT_ColorBuffer, &mesh.colors[0].x, mesh.colors.size() * sizeof(glm::vec4), Buffer::FillType::FT_Single);

		m_buffers.push_back(buffer);
	}
}

void Model::NormalizeSize()
{
	glm::vec3 minBound(std::numeric_limits<float>::max());
	glm::vec3 maxBound(std::numeric_limits<float>::lowest());

	for (const auto& mesh : m_meshes)
	{
		for (const auto& vertex : mesh.vertices)
		{
			minBound = glm::min(minBound, vertex);
			maxBound = glm::max(maxBound, vertex);
		}
	}

	glm::vec3 size = maxBound - minBound;
	float maxExtent = glm::max(size.x, glm::max(size.y, size.z));

	if (maxExtent <= 0.0f) { return; }

	const float targetSize = 2.5f;
	float scale = targetSize / maxExtent;

	glm::vec3 center = (minBound + maxBound) * 0.5f;
	center.y = minBound.y;

	for (auto& mesh : m_meshes)
	{
		for (auto& vertex : mesh.vertices)
		{
			vertex = (vertex - center) * scale;
		}
	}

	m_boundsMin = (minBound - center) * scale;
	m_boundsMax = (maxBound - center) * scale;
}

void Model::SortVertexData(Mesh& newMesh, const Mesh& oldMesh, const std::vector<Face>& faces)
{
	GLuint count = 0;
	std::unordered_map<VertexGroup, GLuint, HashFunction> map;

	for (const auto& face : faces)
	{
		glm::vec3 faceNormal(0.0f, 1.0f, 0.0f);
		bool needsGeneratedNormal = false;

		for (const auto& vertexGroup : face)
		{
			if (vertexGroup.n < 0)
			{
				needsGeneratedNormal = true;
				break;
			}
		}

		if (needsGeneratedNormal)
		{
			const glm::vec3& p0 = oldMesh.vertices[face[0].v];
			const glm::vec3& p1 = oldMesh.vertices[face[1].v];
			const glm::vec3& p2 = oldMesh.vertices[face[2].v];

			glm::vec3 crossProduct = glm::cross(p1 - p0, p2 - p0);

			if (glm::dot(crossProduct, crossProduct) > 0.0f)
			{
				faceNormal = glm::normalize(crossProduct);
			}
		}

		for (const auto& vertexGroup : face)
		{
			if (vertexGroup.n < 0)
			{
				newMesh.vertices.push_back(oldMesh.vertices[vertexGroup.v]);
				newMesh.textureCoordinates.push_back((vertexGroup.t >= 0) ? oldMesh.textureCoordinates[vertexGroup.t] : glm::vec2(0.0f));
				newMesh.normals.push_back(faceNormal);
				newMesh.indices.push_back(count);
				count++;
				continue;
			}

			auto it = map.find(vertexGroup);

			if (it == map.end())
			{
				newMesh.vertices.push_back(oldMesh.vertices[vertexGroup.v]);
				newMesh.textureCoordinates.push_back((vertexGroup.t >= 0) ? oldMesh.textureCoordinates[vertexGroup.t] : glm::vec2(0.0f));
				newMesh.normals.push_back(oldMesh.normals[vertexGroup.n]);
				newMesh.indices.push_back(count);

				map[vertexGroup] = count;
				count++;
			}
			else
			{
				newMesh.indices.push_back(it->second);
			}
		}
	}
}
