#pragma once

#include <glm.hpp>
#include <glad/gl.h>


class Transform
{
public:
	Transform(const glm::mat4& matrix = glm::mat4(1.0f));

	const glm::vec3 GetPosition() const;
	const glm::vec3 GetRotation() const;
	const glm::vec3 GetScale() const;
	const glm::mat4 GetMatrix();

	void SetPosition(GLfloat x, GLfloat y, GLfloat z);
	void SetRotation(GLfloat pitch, GLfloat yaw, GLfloat roll);
	void SetScale(GLfloat x, GLfloat y, GLfloat z);
	void SetIdentity();

private:
	glm::mat4 m_matrix;

	glm::vec3 m_position;
	glm::vec3 m_rotation;
	glm::vec3 m_scale;

	bool m_isDirty;
};

