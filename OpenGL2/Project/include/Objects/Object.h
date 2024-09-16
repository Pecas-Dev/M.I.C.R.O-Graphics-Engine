#pragma once

#include <Transform/Transform.h>
#include <Shader/Shader.h>
#include <Grid/Grid.h>

#include <glm.hpp>
#include <glad/gl.h>


class Object
{
public:
	Object(Grid* parentGrid = nullptr);
	virtual ~Object() = 0 {}

	virtual void Update() = 0;
	virtual void Render(const Shader& shader);
	virtual void SetColor(const glm::vec4& color) = 0;

	void IsTextured(bool isTextured);

	bool IsTextured();

	const glm::vec4& GetColor() const;

	Transform& GetTransform();

protected:
	Transform m_transform;
	Grid* m_parentGrid;

	glm::mat3 m_normal;
	glm::vec4 m_color;

	bool m_isTextured;
};

