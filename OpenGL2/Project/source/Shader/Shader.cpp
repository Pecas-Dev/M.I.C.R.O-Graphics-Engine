#include <Utility/Utility.h>
#include <Shader/Shader.h>

#include <fstream>
#include <iostream>


GLuint Shader::m_vertexShaderID = 0;
GLuint Shader::m_fragmentShaderID = 0;


Shader::Shader()
{
	m_shaderProgramID = 0;
}

Shader::~Shader()
{
	/*glDeleteShader(m_vertexShaderID);
	glDeleteShader(m_fragmentShaderID);*/
}

GLuint Shader::GetShaderProgramID() const
{
	return m_shaderProgramID;
}

bool Shader::Create(const std::string& vertexShaderFilename, const std::string& fragmentShaderFilename)
{
	m_shaderProgramID = glCreateProgram();

	if (m_shaderProgramID == 0)
	{
		std::cout << "Error creating shader program." << std::endl;
		return false;
	}

	if (m_vertexShaderID == 0)
	{
		m_vertexShaderID = glCreateShader(GL_VERTEX_SHADER);

		if (m_vertexShaderID == 0)
		{
			std::cout << "Error creating vertex shader object." << std::endl;
			return false;
		}
	}

	if (m_fragmentShaderID == 0)
	{
		m_fragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

		if (m_fragmentShaderID == 0)
		{
			std::cout << "Error creating fragment shader object." << std::endl;
			return false;
		}
	}

	if (!CompileShaders(vertexShaderFilename, ShaderType::ST_VertexShader))
	{
		return false;
	}

	if (!CompileShaders(fragmentShaderFilename, ShaderType::ST_FragmentShader))
	{
		return false;
	}

	if (!LinkProgram())
	{
		return false;
	}

	return true;
}


bool Shader::CompileShaders(const std::string& filename, ShaderType shaderType)
{
	std::string sourceCode;
	std::string text;

	std::fstream file;

	GLuint shaderID;


	shaderID = (shaderType == ShaderType::ST_VertexShader) ? m_vertexShaderID : m_fragmentShaderID;

	file.open(filename);

	if (!file)
	{
		Utility::AddMessage("Failed to open shader file: " + filename);
		return false;
	}

	while (!file.eof())
	{
		std::getline(file, text);
		sourceCode += text + "\n";
	}

	file.close();

	const GLchar* finalSourceCode = reinterpret_cast<const GLchar*>(sourceCode.c_str());
	glShaderSource(shaderID, 1, &finalSourceCode, nullptr);

	glCompileShader(shaderID);

	GLint errorCode;
	glGetShaderiv(shaderID, GL_COMPILE_STATUS, &errorCode);

	if (errorCode == GL_TRUE)
	{
		Utility::AddMessage("Shader compilation successful!");
	}

	else
	{
		GLchar errorMessage[1000];
		GLsizei bufferSize = 1000;

		glGetShaderInfoLog(shaderID, bufferSize, &bufferSize, errorMessage);
		Utility::AddMessage(errorMessage);
		return false;
	}

	return true;
}

bool Shader::LinkProgram()
{
	glAttachShader(m_shaderProgramID, m_vertexShaderID);
	glAttachShader(m_shaderProgramID, m_fragmentShaderID);
	glLinkProgram(m_shaderProgramID);
	glDetachShader(m_shaderProgramID, m_vertexShaderID);
	glDetachShader(m_shaderProgramID, m_fragmentShaderID);

	GLint errorCode;
	glGetProgramiv(m_shaderProgramID, GL_LINK_STATUS, &errorCode);

	if (errorCode == GL_TRUE)
	{
		Utility::AddMessage("Shader linking successful!");
	}

	else
	{
		GLchar errorMessage[1000];
		GLsizei bufferSize = 1000;

		glGetProgramInfoLog(m_shaderProgramID, bufferSize, &bufferSize, errorMessage);
		Utility::AddMessage(errorMessage);
		return false;
	}

	return true;
}

bool Shader::SendUniformData(const std::string& uniformName, GLint data) const
{
	GLint ID = glGetUniformLocation(m_shaderProgramID, uniformName.c_str());

	if (ID == -1)
	{
		//std::cout << "Shader variable " << uniformName << " not found or not used." << std::endl;
		return false;
	}

	glUniform1i(ID, data);

	return true;
}

bool Shader::SendUniformData(const std::string& uniformName, GLuint data) const
{
	GLint ID = glGetUniformLocation(m_shaderProgramID, uniformName.c_str());

	if (ID == -1)
	{
		//std::cout << "Shader variable " << uniformName << " not found or not used." << std::endl;
		return false;
	}

	glUniform1ui(ID, data);

	return true;
}

bool Shader::SendUniformData(const std::string& uniformName, GLfloat data) const
{
	GLint ID = glGetUniformLocation(m_shaderProgramID, uniformName.c_str());

	if (ID == -1)
	{
		//std::cout << "Shader variable " << uniformName << " not found or not used." << std::endl;
		return false;
	}

	glUniform1f(ID, data);

	return true;
}

bool Shader::SendUniformData(const std::string& uniformName, GLfloat x, GLfloat y) const
{
	GLint ID = glGetUniformLocation(m_shaderProgramID, uniformName.c_str());

	if (ID == -1)
	{
		//std::cout << "Shader variable " << uniformName << " not found or not used." << std::endl;
		return false;
	}

	glUniform2f(ID, x, y);

	return true;
}

bool Shader::SendUniformData(const std::string& uniformName, GLfloat x, GLfloat y, GLfloat z) const
{
	GLint ID = glGetUniformLocation(m_shaderProgramID, uniformName.c_str());

	if (ID == -1)
	{
		//std::cout << "Shader variable " << uniformName << " not found or not used." << std::endl;
		return false;
	}

	glUniform3f(ID, x, y, z);

	return true;
}

bool Shader::SendUniformData(const std::string& uniformName, GLfloat x, GLfloat y, GLfloat z, GLfloat w) const
{
	GLint ID = glGetUniformLocation(m_shaderProgramID, uniformName.c_str());

	if (ID == -1)
	{
		//std::cout << "Shader variable " << uniformName << " not found or not used." << std::endl;
		return false;
	}

	glUniform4f(ID, x, y, z, w);

	return true;
}

bool Shader::SendUniformData(const std::string& uniformName, const glm::mat4& data) const
{
	GLint ID = glGetUniformLocation(m_shaderProgramID, uniformName.c_str());

	if (ID == -1)
	{
		//std::cout << "Shader variable " << uniformName << " not found or not used." << std::endl;
		return false;
	}

	glUniformMatrix4fv(ID, 1, GL_FALSE, &data[0][0]);

	return true;
}

bool Shader::SendUniformData(const std::string& uniformName, const glm::mat3& data) const
{
	GLint ID = glGetUniformLocation(m_shaderProgramID, uniformName.c_str());

	if (ID == -1)
	{
		//std::cout << "Shader variable " << uniformName << " not found or not used." << std::endl;
		return false;
	}

	glUniformMatrix3fv(ID, 1, GL_TRUE, &data[0][0]);

	return true;
}

void Shader::UseShader()
{
	glUseProgram(m_shaderProgramID);
}

void Shader::Destroy()
{
	glDeleteProgram(m_shaderProgramID);
}
