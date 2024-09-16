#include <InputSystem/Input.h>
#include <Grid/Grid.h>

#include <gtc/matrix_transform.hpp>


Grid::Grid()
{
	const auto size = 10;
	const auto quadrants = 4;
	const auto verticesPerLine = 2;

	const auto bytesPerLineVertex = verticesPerLine * static_cast<GLuint>(Buffer::ComponentType::CT_XYZ) * sizeof(GLint);
	const auto bytesPerLineColor = verticesPerLine * static_cast<GLuint>(Buffer::ComponentType::CT_RGBA) * sizeof(GLfloat);

	const auto bytesVertexVBO = (size + 1)  * quadrants * bytesPerLineVertex;
	const auto bytesColorVBO = (size + 1) * quadrants * bytesPerLineColor;

	m_buffer.CreateBuffer((size + 1) * quadrants * verticesPerLine, false);

	m_buffer.FillVBO(Buffer::VBOType::VBOT_VertexBuffer, (GLint*)nullptr, bytesVertexVBO, Buffer::FillType::FT_Multiple);
	m_buffer.FillVBO(Buffer::VBOType::VBOT_ColorBuffer, (GLfloat*)nullptr, bytesColorVBO, Buffer::FillType::FT_Multiple);

	auto offsetColor = 0U;
	auto offsetVertex = 0U;


	for (int i = 0; i <= size; i++)
	{
		GLint vertices[] = { -size + i, 0,  size,
							 -size + i, 0, -size };

		GLfloat colors[] = { 0.5f, 0.5f, 0.5f, 1.0f,
							 0.5f, 0.5f, 0.5f, 1.0f };

		m_buffer.AppendVBO(Buffer::VBOType::VBOT_VertexBuffer, vertices, sizeof(vertices), offsetVertex);
		m_buffer.AppendVBO(Buffer::VBOType::VBOT_ColorBuffer, colors, sizeof(colors), offsetColor);

		offsetVertex += bytesPerLineVertex;
		offsetColor += bytesPerLineColor;
	}

	for (int i = 1; i < size + 1; i++)
	{
		GLint vertices[] = { 0 + i, 0,  size,
							 0 + i, 0, -size };

		GLfloat colors[] = { 0.5f, 0.5f, 0.5f, 1.0f,
							 0.5f, 0.5f, 0.5f, 1.0f };

		m_buffer.AppendVBO(Buffer::VBOType::VBOT_VertexBuffer, vertices, sizeof(vertices), offsetVertex);
		m_buffer.AppendVBO(Buffer::VBOType::VBOT_ColorBuffer, colors, sizeof(colors), offsetColor);

		offsetVertex += bytesPerLineVertex;
		offsetColor += bytesPerLineColor;
	}

	for (int i = 0; i <= size; i++)
	{
		GLint vertices[] = { -size, 0, -size + i,
							  size, 0, -size + i };

		GLfloat colors[] = { 0.5f, 0.5f, 0.5f, 1.0f,
							 0.5f, 0.5f, 0.5f, 1.0f };

		m_buffer.AppendVBO(Buffer::VBOType::VBOT_VertexBuffer, vertices, sizeof(vertices), offsetVertex);
		m_buffer.AppendVBO(Buffer::VBOType::VBOT_ColorBuffer, colors, sizeof(colors), offsetColor);

		offsetVertex += bytesPerLineVertex;
		offsetColor += bytesPerLineColor;
	}

	for (int i = 1; i < size + 1; i++)
	{
		GLint vertices[] = { -size, 0, 0 + i,
							  size, 0, 0 + i };

		GLfloat colors[] = { 0.5f, 0.5f, 0.5f, 1.0f,
							 0.5f, 0.5f, 0.5f, 1.0f };

		m_buffer.AppendVBO(Buffer::VBOType::VBOT_VertexBuffer, vertices, sizeof(vertices), offsetVertex);
		m_buffer.AppendVBO(Buffer::VBOType::VBOT_ColorBuffer, colors, sizeof(colors), offsetColor);

		offsetVertex += bytesPerLineVertex;
		offsetColor += bytesPerLineColor;
	}

	m_position = glm::vec3(0.0f);
	m_rotation = glm::vec3(0.0f);
}

void Grid::Render(const Shader& shader)
{
	shader.SendUniformData("model", m_transform.GetMatrix());
	shader.SendUniformData("isTextured", false);

	m_buffer.LinkVBO(shader, "vertexIn", Buffer::VBOType::VBOT_VertexBuffer, Buffer::ComponentType::CT_XYZ, Buffer::DataType::DT_Int);
	m_buffer.LinkVBO(shader, "colorIn", Buffer::VBOType::VBOT_ColorBuffer, Buffer::ComponentType::CT_RGBA, Buffer::DataType::DT_Float);

	m_buffer.Render(Buffer::DrawType::DwT_Lines);
}

void Grid::MoveGrid(Grid& grid)
{
	if (Input::Instance()->IsLeftButtonClicked())
	{
		auto rotation = grid.GetTransform().GetRotation();
		rotation.x += Input::Instance()->GetMouseMotionY();
		rotation.y += Input::Instance()->GetMouseMotionX();
		grid.GetTransform().SetRotation(rotation.x, rotation.y, rotation.z);
	}
}

Transform& Grid::GetTransform()
{
	return m_transform;
}
