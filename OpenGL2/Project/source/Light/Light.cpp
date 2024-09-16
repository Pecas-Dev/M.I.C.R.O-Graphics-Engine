#include <InputSystem/Input.h>
#include <Shader/Shader.h>
#include <Light/Light.h>

#include <gtc/matrix_transform.hpp>


Light::Light()
{
	m_ambient = glm::vec3(1.0f);
	m_diffuse = glm::vec3(1.0f);
	m_specular = glm::vec3(1.0f);
	m_transform.SetPosition(0.0f, 5.0f, 0.0f);

	GLfloat vertices[] = { 0.0f, 0.0f, 0.0f };
	GLfloat colors[] = { 1.0f, 1.0f, 1.0f };

	m_buffer.CreateBuffer(1);

	m_buffer.FillVBO(Buffer::VBOType::VBOT_VertexBuffer, vertices, sizeof(vertices), Buffer::FillType::FT_Single);
	m_buffer.FillVBO(Buffer::VBOType::VBOT_ColorBuffer, colors, sizeof(colors), Buffer::FillType::FT_Single);
}

Light::~Light()
{
	m_buffer.DestroyBuffer();
}

void Light::MoveUp()
{
	auto position = m_transform.GetPosition();
	position.y += m_speed;
	m_transform.SetPosition(position.x, position.y, position.z);
}

void Light::MoveDown()
{
	auto position = m_transform.GetPosition();
	position.y -= m_speed;
	m_transform.SetPosition(position.x, position.y, position.z);

}

void Light::MoveLeft()
{
	auto position = m_transform.GetPosition();
	position.x -= m_speed;
	m_transform.SetPosition(position.x, position.y, position.z);
}

void Light::MoveRight()
{
	auto position = m_transform.GetPosition();
	position.x += m_speed;
	m_transform.SetPosition(position.x, position.y, position.z);
}

void Light::MoveForward()
{
	auto position = m_transform.GetPosition();
	position.z -= m_speed;
	m_transform.SetPosition(position.x, position.y, position.z);
}

void Light::MoveBackward()
{
	auto position = m_transform.GetPosition();
	position.z += m_speed;
	m_transform.SetPosition(position.x, position.y, position.z);
}

void Light::SetSpeed(GLfloat speed)
{
	m_speed = speed;
}

void Light::Render(const Shader& shader)
{
	m_model = glm::mat4(1.0f);
	m_model = glm::translate(m_model, m_transform.GetPosition());

	shader.SendUniformData("model", m_model);
	shader.SendUniformData("isTextured", true);
	//shader.SendUniformData("isLit", false);

	m_buffer.LinkVBO(shader, "vertexIn", Buffer::VBOType::VBOT_VertexBuffer, Buffer::ComponentType::CT_XYZ, Buffer::DataType::DT_Float);
	m_buffer.LinkVBO(shader, "colorIn", Buffer::VBOType::VBOT_ColorBuffer, Buffer::ComponentType::CT_RGB, Buffer::DataType::DT_Float);

	glPointSize(20.0f);

	m_buffer.Render(Buffer::DrawType::DwT_Points);
}

void Light::SendToShader(const Shader& shader)
{
	auto position = m_transform.GetPosition();

	shader.SendUniformData("light.position", position.x, position.y, position.z);
	shader.SendUniformData("light.ambient", m_ambient.r, m_ambient.g, m_ambient.b);
	shader.SendUniformData("light.diffuse", m_diffuse.r, m_diffuse.g, m_diffuse.b);
	shader.SendUniformData("light.specular", m_specular.r, m_specular.g, m_specular.b);
}

void Light::MoveLight(Light& light)
{
	if (Input::Instance()->IsKeyPressed())
	{
		if (Input::Instance()->GetKeyDown() == 'w')
		{
			light.MoveForward();
		}

		else if (Input::Instance()->GetKeyDown() == 's')
		{
			light.MoveBackward();
		}

		else if (Input::Instance()->GetKeyDown() == 'a')
		{
			light.MoveLeft();
		}

		else if (Input::Instance()->GetKeyDown() == 'd')
		{
			light.MoveRight();
		}

		else if (Input::Instance()->GetKeyDown() == (char)82)
		{
			light.MoveUp();
		}

		else if (Input::Instance()->GetKeyDown() == (char)81)
		{
			light.MoveDown();
		}
	}
}