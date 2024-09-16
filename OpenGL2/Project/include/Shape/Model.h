#pragma once

#include <Material/Material.h>
#include <Objects/Object.h>
#include <Buffer/Buffer.h>

#include <string>
#include <vector>


struct Mesh 
{
	std::string name;
	std::string materialName;

	std::vector<glm::vec4> colors;
	std::vector<glm::vec3> vertices;
	std::vector<glm::vec3> normals;
	std::vector<glm::vec2> textureCoordinates;

	std::vector<GLuint> indices;
};

struct VertexGroup
{
	int v = -1;
	int t = -1;
	int n = -1;

	bool operator==(const VertexGroup& other) const
	{
		return (v == other.v && t == other.t && n == other.n);
	}
};


typedef std::vector<VertexGroup> Face;


class Model : public Object
{
public:
	Model(Grid* parentGrid = nullptr);
	
	bool Load(const std::string& filename);

	virtual void Update() override {};
	virtual void Render(const Shader& shader) override;

	virtual void SetColor(const glm::vec4& color) override;

private:
	void SortVertexData(Mesh& newMesh, const Mesh& oldMesh, const std::vector<Face>& faces);
	void FillBuffers();

	std::vector<Mesh> m_meshes;
	std::vector<Buffer> m_buffers;
	std::vector<Material> m_materials;
};

class HashFunction
{
public:
	size_t operator()(const VertexGroup& v) const
	{
		return v.v + v.t + v.n;
	}
};
