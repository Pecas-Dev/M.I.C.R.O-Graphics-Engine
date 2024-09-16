#include <Shape/Cube.h>
#include <Shader/Shader.h>
#include <InputSystem/Input.h>


Cube::Cube(const std::string& textureFilename, Grid* parentGrid) : Object(parentGrid)
{
	GLfloat vertices[] = { -0.5f,  0.5f,  0.5f,
							0.5f,  0.5f,  0.5f,
							0.5f, -0.5f,  0.5f,
						   -0.5f, -0.5f,  0.5f,

							0.5f,  0.5f, -0.5f,
						   -0.5f,  0.5f, -0.5f,
						   -0.5f, -0.5f, -0.5f,
							0.5f, -0.5f, -0.5f,

						   -0.5f,  0.5f, -0.5f,
						   -0.5f,  0.5f,  0.5f,
						   -0.5f, -0.5f,  0.5f,
						   -0.5f, -0.5f, -0.5f,

							0.5f,  0.5f,  0.5f,
							0.5f,  0.5f, -0.5f,
							0.5f, -0.5f, -0.5f,
							0.5f, -0.5f,  0.5f,

						   -0.5f,  0.5f, -0.5f,
							0.5f,  0.5f, -0.5f,
							0.5f,  0.5f,  0.5f,
						   -0.5f,  0.5f,  0.5f,

						   -0.5f, -0.5f,  0.5f,
							0.5f, -0.5f,  0.5f,
							0.5f, -0.5f, -0.5f,
						   -0.5f, -0.5f, -0.5f };


	GLfloat colors[] = { 1.0f, 1.0f, 1.0f, 1.0f,
						 1.0f, 1.0f, 1.0f, 1.0f,
						 1.0f, 1.0f, 1.0f, 1.0f,
						 1.0f, 1.0f, 1.0f, 1.0f,

						 1.0f, 1.0f, 1.0f, 1.0f,
						 1.0f, 1.0f, 1.0f, 1.0f,
						 1.0f, 1.0f, 1.0f, 1.0f,
						 1.0f, 1.0f, 1.0f, 1.0f,

						 1.0f, 1.0f, 1.0f, 1.0f,
						 1.0f, 1.0f, 1.0f, 1.0f,
						 1.0f, 1.0f, 1.0f, 1.0f,
						 1.0f, 1.0f, 1.0f, 1.0f,

						 1.0f, 1.0f, 1.0f, 1.0f,
						 1.0f, 1.0f, 1.0f, 1.0f,
						 1.0f, 1.0f, 1.0f, 1.0f,
						 1.0f, 1.0f, 1.0f, 1.0f,

						 1.0f, 1.0f, 1.0f, 1.0f,
						 1.0f, 1.0f, 1.0f, 1.0f,
						 1.0f, 1.0f, 1.0f, 1.0f,
						 1.0f, 1.0f, 1.0f, 1.0f,

						 1.0f, 1.0f, 1.0f, 1.0f,
						 1.0f, 1.0f, 1.0f, 1.0f,
						 1.0f, 1.0f, 1.0f, 1.0f,
						 1.0f, 1.0f, 1.0f, 1.0f };


	GLfloat UVs[] = { 0.0f, 1.0f, 1.0f, 1.0f,
					  1.0f, 0.0f, 0.0f, 0.0f,

					  0.0f, 1.0f, 1.0f, 1.0f,
					  1.0f, 0.0f, 0.0f, 0.0f,

					  0.0f, 1.0f, 1.0f, 1.0f,
					  1.0f, 0.0f, 0.0f, 0.0f,

					  0.0f, 1.0f, 1.0f, 1.0f,
					  1.0f, 0.0f, 0.0f, 0.0f,

					  0.0f, 1.0f, 1.0f, 1.0f,
					  1.0f, 0.0f, 0.0f, 0.0f,

					  0.0f, 1.0f, 1.0f, 1.0f,
					  1.0f, 0.0f, 0.0f, 0.0f };



	GLfloat normals[] = { 0.0f,  0.0f,  1.0f,  0.0f,  0.0f,  1.0f,
						  0.0f,  0.0f,  1.0f,  0.0f,  0.0f,  1.0f,

						   0.0f,  0.0f, -1.0f,  0.0f,  0.0f, -1.0f,
						   0.0f,  0.0f, -1.0f,  0.0f,  0.0f, -1.0f,

						  -1.0f,  0.0f,  0.0f, -1.0f,  0.0f,  0.0f,
						  -1.0f,  0.0f,  0.0f, -1.0f,  0.0f,  0.0f,

						   1.0f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f,
						   1.0f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f,

						   0.0f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,
						   0.0f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,

						   0.0f, -1.0f,  0.0f,  0.0f, -1.0f,  0.0f,
						   0.0f, -1.0f,  0.0f,  0.0f, -1.0f,  0.0f };


	GLuint indices[] = { 0,  1,  3,  3,  1,  2,
						 4,  5,  7,  7,  5,  6,
						 8,  9, 11, 11,  9, 10,
						12, 13, 15, 15, 13, 14,
						16, 17, 19, 19, 17, 18,
						20, 21, 23, 23, 21, 22 };


	m_buffer.CreateBuffer(36, true);
	m_buffer.FillEBO(indices, sizeof(indices), Buffer::FillType::FT_Single);
	m_buffer.FillVBO(Buffer::VBOType::VBOT_VertexBuffer, vertices, sizeof(vertices), Buffer::FillType::FT_Single);
	m_buffer.FillVBO(Buffer::VBOType::VBOT_ColorBuffer, colors, sizeof(colors), Buffer::FillType::FT_Single);
	m_buffer.FillVBO(Buffer::VBOType::VBOT_TextureBuffer, UVs, sizeof(UVs), Buffer::FillType::FT_Single);
	m_buffer.FillVBO(Buffer::VBOType::VBOT_NormalBuffer, normals, sizeof(normals), Buffer::FillType::FT_Single);

	m_buffer.LinkEBO();

	m_texture.Load("Assets/Textures/" + textureFilename);

	m_material.SetShininess(50.0f);
	m_material.SetAmbient(glm::vec3(0.4f, 0.4f, 0.4f));
	m_material.SetDiffuse(glm::vec3(0.1f, 0.7f, 0.2f));
	m_material.SetSpecular(glm::vec3(0.8f, 0.8f, 0.8f));
}

Cube::~Cube()
{
	m_buffer.DestroyBuffer();
}

void Cube::Render(const Shader& shader)
{
	Object::Render(shader);

	shader.SendUniformData("isTextured", m_isTextured);
	//shader.SendUniformData("isLit", false);

	m_material.SendToShader(shader);

	m_buffer.LinkVBO(shader, "vertexIn", Buffer::VBOType::VBOT_VertexBuffer, Buffer::ComponentType::CT_XYZ, Buffer::DataType::DT_Float);
	m_buffer.LinkVBO(shader, "colorIn", Buffer::VBOType::VBOT_ColorBuffer, Buffer::ComponentType::CT_RGBA, Buffer::DataType::DT_Float);
	m_buffer.LinkVBO(shader, "textureIn", Buffer::VBOType::VBOT_TextureBuffer, Buffer::ComponentType::CT_UV, Buffer::DataType::DT_Float);
	m_buffer.LinkVBO(shader, "normalIn", Buffer::VBOType::VBOT_NormalBuffer, Buffer::ComponentType::CT_XYZ, Buffer::DataType::DT_Float);

	if (m_isTextured)
	{
		m_texture.Bind();
	}

	m_buffer.Render(Buffer::DrawType::DwT_Triangles);
	m_texture.Unbind();
}

void Cube::SetColor(const glm::vec4& color)
{
	std::vector<glm::vec4> colors;

	for(size_t i = 0; i < 24; i++)
	{
		colors.push_back(color);
	}

	m_buffer.FillVBO(Buffer::VBOType::VBOT_ColorBuffer, colors.data(), colors.size() * sizeof(glm::vec4), Buffer::FillType::FT_Multiple);

	m_color = color;
}