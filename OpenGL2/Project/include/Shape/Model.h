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
	std::vector<glm::vec3> tangents;
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

	void RenderDepth(const Shader& shader);

	virtual void SetColor(const glm::vec4& color) override;

	bool HasGeometry() const;
	bool HasTexturedMaterial() const;
	int GetMeshCount() const;
	int GetVertexCount() const;
	int GetTriangleCount() const;

	const std::string& GetSourceFile() const;
	std::string GetDisplayName() const;

	bool& Visible();

	const glm::vec3& GetBoundsMin() const;
	const glm::vec3& GetBoundsMax() const;

	void ApplyMaterialPreset(const std::string& name, const glm::vec3& tint, float roughness, float metallic);
	void ClearMaterialPreset();
	const std::string& GetMaterialPresetName() const;

private:
	bool LoadOBJ(const std::string& filename, const std::string& directory);
	bool LoadWithAssimp(const std::string& filename, const std::string& directory);

	void SortVertexData(Mesh& newMesh, const Mesh& oldMesh, const std::vector<Face>& faces);
	void NormalizeSize();
	void ComputeTangents(Mesh& mesh);
	void FillBuffers();

	std::vector<Mesh> m_meshes;
	std::vector<Buffer> m_buffers;
	std::vector<Material> m_materials;

	std::string m_sourceFile;

	bool m_visible = true;

	glm::vec3 m_boundsMin = glm::vec3(-0.5f);
	glm::vec3 m_boundsMax = glm::vec3(0.5f);

	std::string m_presetName;
	glm::vec3 m_presetTint = glm::vec3(1.0f);
	float m_presetRoughness = 0.5f;
	float m_presetMetallic = 0.0f;
};

class HashFunction
{
public:
	size_t operator()(const VertexGroup& v) const
	{
		return v.v + v.t + v.n;
	}
};
