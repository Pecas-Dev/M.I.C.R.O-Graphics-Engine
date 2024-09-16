#include <InputSystem/Input.h>
#include <Utility/Utility.h>
#include <Shape/Model.h>

#include <fstream>
#include <iostream>
#include <unordered_map>


Model::Model(Grid* parentGrid) : Object(parentGrid)
{
	m_meshes.reserve(10000);
	m_buffers.reserve(10000);
}

bool Model::Load(const std::string& filename)
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
	subStrings.reserve(10000);


	while (!file.eof())
	{
		std::getline(file, line);
		subStrings.clear();

		if (!line.empty() && line[0] != '#')
		{
			Utility::ParseString(line, subStrings, ' ');

			if (subStrings[0] == "v")
			{
				rawMesh.vertices.push_back(glm::vec3(std::stof(subStrings[1]), std::stof(subStrings[2]), std::stof(subStrings[3])));
				continue;
			}

			if (subStrings[0] == "vn")
			{
				rawMesh.normals.push_back(glm::vec3(std::stof(subStrings[1]), std::stof(subStrings[2]), std::stof(subStrings[3])));
				continue;
			}

			if (subStrings[0] == "vt")
			{
				rawMesh.textureCoordinates.push_back(glm::vec2(std::stof(subStrings[1]), std::stof(subStrings[2])));
				continue;
			}

			if (subStrings[0] == "f")
			{
				std::vector<VertexGroup> vertexGroups;
				vertexGroups.reserve(subStrings.size() - 1);

				for (size_t i = 1; i < subStrings.size(); i++)
				{
					std::vector<std::string> numbers;
					numbers.reserve(10000);
					Utility::ParseString(subStrings[i], numbers, '/');

					VertexGroup vertexGroup;
					vertexGroup.v = std::stoi(numbers[0]) - 1;
					vertexGroup.t = std::stoi(numbers[1]) - 1;
					vertexGroup.n = std::stoi(numbers[2]) - 1;
					vertexGroups.push_back(vertexGroup);
				}

				for (size_t i = 1; i < vertexGroups.size() - 1; i++)
				{
					Face face;
					face.push_back(vertexGroups[0]);
					face.push_back(vertexGroups[i]);
					face.push_back(vertexGroups[i + 1]);
					faces.push_back(face);
				}

				continue;
			}

			if (subStrings[0] == "usemtl")
			{
				if (!m_materials.empty())
				{
					for (const auto& material : m_materials)
					{
						if (material.GetName() == subStrings[1])
						{
							lastMaterialName = subStrings[1];
							break;
						}
					}
				}

				continue;
			}

			if (subStrings[0] == "mtllib")
			{
				Material material;
				material.Load(subStrings[1], m_materials);
				continue;
			}

			if (subStrings[0] == "g" || subStrings[0] == "o")
			{
				if (!faces.empty())
				{
					Mesh mesh;
					mesh.name = lastName;
					mesh.materialName = lastMaterialName;
					SortVertexData(mesh, rawMesh, faces);
					m_meshes.push_back(mesh);
				}

				lastName = subStrings[1];
				faces.clear();
				continue;
			}
		}
	}

	file.close();

	if (!faces.empty())
	{
		Mesh mesh;
		mesh.name = lastName;
		mesh.materialName = lastMaterialName;
		SortVertexData(mesh, rawMesh, faces);
		m_meshes.push_back(mesh);
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

		for (auto mat : m_materials)
		{
			if (mat.GetName() == m_meshes[count].materialName)
			{
				mat.SendToShader(shader);

				if (mat.IsTextured())
				{
					if (m_isTextured) { shader.SendUniformData("isTextured", true); }
					mat.GetDiffuseMap().Bind();
				}
				else
				{
					shader.SendUniformData("isTextured", false);
				}

				break;
			}
		}

		count++;

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

void Model::FillBuffers()
{
	for (auto& mesh : m_meshes)
	{
		Buffer buffer;
		buffer.CreateBuffer(mesh.indices.size(), true);

		buffer.FillEBO(&mesh.indices[0], mesh.indices.size() * sizeof(GLuint), Buffer::FillType::FT_Single);
		buffer.FillVBO(Buffer::VBOType::VBOT_VertexBuffer, &mesh.vertices[0].x, mesh.vertices.size() * sizeof(glm::vec3), Buffer::FillType::FT_Single);
		buffer.FillVBO(Buffer::VBOType::VBOT_TextureBuffer, &mesh.textureCoordinates[0].x, mesh.textureCoordinates.size() * sizeof(glm::vec2), Buffer::FillType::FT_Single);
		buffer.FillVBO(Buffer::VBOType::VBOT_NormalBuffer, &mesh.normals[0].x, mesh.normals.size() * sizeof(glm::vec3), Buffer::FillType::FT_Single);

		for (const auto& vertex : mesh.vertices)
		{
			glm::vec4 color(1.0f, 1.0f, 1.0f, 1.0f);
			mesh.colors.push_back(color);
		}

		buffer.FillVBO(Buffer::VBOType::VBOT_ColorBuffer, &mesh.colors[0].x, mesh.colors.size() * sizeof(glm::vec4), Buffer::FillType::FT_Single);

		m_buffers.push_back(buffer);
	}
}

void Model::SortVertexData(Mesh& newMesh, const Mesh& oldMesh, const std::vector<Face>& faces)
{
	GLuint count = 0;
	std::unordered_map<VertexGroup, GLuint, HashFunction> map;

	for (const auto& face : faces)
	{
		for (const auto& vertexGroup : face)
		{
			auto it = map.find(vertexGroup);

			if (it == map.end())
			{
				newMesh.vertices.push_back(oldMesh.vertices[vertexGroup.v]);
				newMesh.textureCoordinates.push_back(oldMesh.textureCoordinates[vertexGroup.t]);
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