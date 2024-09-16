#pragma once

#include <Transform/Transform.h>
#include <Buffer/Buffer.h>
#include <Shader/Shader.h>

#include <glad/gl.h>
#include <glm.hpp>


class Grid
{
public:
	Grid();

	void Update() {}
	void Render(const Shader& shader);
	void MoveGrid(Grid& grid);

	Transform& GetTransform();

private:
	Buffer m_buffer;
	Transform m_transform;

	glm::mat4 m_model;
	glm::vec3 m_position;
	glm::vec3 m_rotation;
};