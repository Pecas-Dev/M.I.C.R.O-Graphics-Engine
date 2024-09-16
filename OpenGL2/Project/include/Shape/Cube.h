#pragma once

#include <Material/Material.h>
#include <Texture/Texture.h>
#include <Objects/Object.h>
#include <Buffer/Buffer.h>

#include <string>


class Cube : public Object
{
public:
	Cube(const std::string& textureFilename, Grid* parentGrid = nullptr);
	~Cube();

	virtual void Update() override {};
	virtual void Render(const Shader& shader) override;

	virtual void SetColor(const glm::vec4& color) override;

private:
	Buffer m_buffer;
	Texture m_texture;
	Material m_material;
};

