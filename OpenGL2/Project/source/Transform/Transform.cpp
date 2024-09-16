#include <Transform/Transform.h>

#include <gtc/matrix_transform.hpp>


Transform::Transform(const glm::mat4& matrix)
{
	m_position = glm::vec3(0.0f);
	m_rotation = glm::vec3(0.0f);
	m_scale = glm::vec3(1.0f);
	m_matrix = matrix;

	m_isDirty = true;
}

const glm::vec3 Transform::GetPosition() const
{
	return m_position;
}

const glm::vec3 Transform::GetRotation() const
{
	return m_rotation;
}

const glm::vec3 Transform::GetScale() const
{
	return m_scale;
}

const glm::mat4 Transform::GetMatrix()
{
	if (m_isDirty)
	{
		m_matrix = glm::mat4(1.0f);
		m_matrix = glm::translate(m_matrix, m_position);
		m_matrix = glm::rotate(m_matrix, glm::radians(m_rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
		m_matrix = glm::rotate(m_matrix, glm::radians(m_rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
		m_matrix = glm::rotate(m_matrix, glm::radians(m_rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));
		m_matrix = glm::scale(m_matrix, m_scale);

		m_isDirty = false;
	}

	return m_matrix;
}

void Transform::SetPosition(GLfloat x, GLfloat y, GLfloat z)
{
	m_position.x = x;
	m_position.y = y;
	m_position.z = z;
	m_isDirty = true;
}

void Transform::SetRotation(GLfloat pitch, GLfloat yaw, GLfloat roll)
{
	m_rotation.x = pitch;
	m_rotation.y = yaw;
	m_rotation.z = roll;
	m_isDirty = true;
}

void Transform::SetScale(GLfloat x, GLfloat y, GLfloat z)
{
	m_scale.x = x;
	m_scale.y = y;
	m_scale.z = z;
	m_isDirty = true;
}

void Transform::SetIdentity()
{
	m_position = glm::vec3(0.0f);
	m_rotation = glm::vec3(0.0f);
	m_scale = glm::vec3(1.0f);
	m_isDirty = true;
}
