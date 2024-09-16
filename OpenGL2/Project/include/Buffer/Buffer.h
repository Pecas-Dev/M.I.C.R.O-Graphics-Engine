#pragma once

#include <Shader/Shader.h>

#include <glad/gl.h>

#include <string>


class Buffer
{
public:
	Buffer();

	enum class VBOType { VBOT_VertexBuffer, VBOT_ColorBuffer, VBOT_TextureBuffer, VBOT_NormalBuffer };
	enum class ComponentType { CT_XY = 2, CT_XYZ = 3, CT_RGB = 3, CT_RGBA = 4, CT_UV = 2 };
	enum class FillType { FT_Single = GL_STATIC_DRAW, FT_Multiple = GL_DYNAMIC_DRAW };
	enum class DataType { DT_Int = GL_INT, DT_Float = GL_FLOAT, DT_UnsignedInt = GL_UNSIGNED_INT };
	enum class DrawType { DwT_Points = GL_POINTS, DwT_Triangles = GL_TRIANGLES, DwT_Lines = GL_LINES, };


	void CreateBuffer(GLuint totalVertices, bool hasEBO = false);
	void FillVBO(VBOType vboType, const void* data, GLsizeiptr bufferSize, FillType fillType);
	void LinkVBO(const Shader& shader, const std::string& attribute, VBOType vboType, ComponentType componentType, DataType dataType);
	void AppendVBO(VBOType vboType,const void* data, GLsizeiptr bufferSize, GLuint offset);
	void FillEBO(const GLuint* data, GLsizeiptr bufferSize, FillType fill = FillType::FT_Single);
	void LinkEBO();
	void Render(DrawType drawType);
	void DestroyBuffer();

private:
	bool m_hasEBO;

	GLuint m_VAO;
	GLuint m_EBO;

	GLuint m_vertexVBO;
	GLuint m_colorVBO;
	GLuint m_textureVBO;
	GLuint m_normalVBO;

	GLuint m_totalVertices;
};
