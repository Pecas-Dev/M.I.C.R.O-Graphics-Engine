#include <Objects/Object.h>


Object::Object(Grid* parentGrid)
{
	m_isTextured = false;
	m_parentGrid = parentGrid;
	m_normal = glm::mat3(1.0f);
	m_color = glm::vec4(1.0f);
}

void Object::Render(const Shader& shader)
{
	m_normal = glm::inverse(glm::mat3(m_transform.GetMatrix()));

	if (m_parentGrid)
	{
		shader.SendUniformData("model", m_parentGrid->GetTransform().GetMatrix() * m_transform.GetMatrix());
	}
	else
	{
		shader.SendUniformData("model", m_transform.GetMatrix());
	}

	shader.SendUniformData("normal", m_normal);
}

void Object::IsTextured(bool isTextured)
{
	m_isTextured = isTextured;
}

bool Object::IsTextured()
{
	return m_isTextured;
}

const glm::vec4& Object::GetColor() const
{
	return m_color;
}

Transform& Object::GetTransform()
{
	return m_transform;
}
