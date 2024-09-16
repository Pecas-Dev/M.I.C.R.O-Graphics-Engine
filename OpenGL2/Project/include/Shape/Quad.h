#pragma once

#include <Material/Material.h>
#include <Texture/Texture.h>
#include <Objects/Object.h>
#include <Buffer/Buffer.h>

#include <string>


class Quad : public Object
{
public:
	Quad(const std::string& textureFilename, Grid* parentGrid = nullptr);
	~Quad();

	virtual void Update() override {};
	virtual void Render(const Shader& shader) override;

	virtual void SetColor(const glm::vec4& color) override;

private:
	Buffer m_buffer;	
	Texture m_texture;
	Material m_material;

	glm::vec4 m_color;
};
