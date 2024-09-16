#pragma once

#include <glad/gl.h>
#include <glm.hpp>

#include <string>


class Shader
{
public:
	Shader();
	~Shader();

	enum class ShaderType { ST_VertexShader, ST_FragmentShader };

	GLuint GetShaderProgramID() const;

	bool Create(const std::string& vertexShaderFilename, const std::string& fragmentShaderFilename);

	bool SendUniformData(const std::string& uniformName, GLint data) const;
	bool SendUniformData(const std::string& uniformName, GLuint data) const;
	bool SendUniformData(const std::string& uniformName, GLfloat data) const;
	
	bool SendUniformData(const std::string& uniformName, GLfloat x, GLfloat y) const;
	bool SendUniformData(const std::string& uniformName, GLfloat x, GLfloat y, GLfloat z) const;
	bool SendUniformData(const std::string& uniformName, GLfloat x, GLfloat y, GLfloat z, GLfloat w) const;
	
	bool SendUniformData(const std::string& uniformName, const glm::mat4& data) const;
	bool SendUniformData(const std::string& uniformName, const glm::mat3& data) const;

	void UseShader();
	void Destroy();

private:
	bool LinkProgram();
	bool CompileShaders(const std::string& filename, ShaderType shaderType);

	GLuint m_shaderProgramID;

	static GLuint m_vertexShaderID;
	static GLuint m_fragmentShaderID;
};
