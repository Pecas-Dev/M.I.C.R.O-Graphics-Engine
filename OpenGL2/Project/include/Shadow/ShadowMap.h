#pragma once

#include <Shader/Shader.h>

#include <glm.hpp>
#include <glad/gl.h>


class ShadowMap
{
public:
	ShadowMap();

	bool Create(GLsizei resolution = 2048);

	void BeginDepthPass(const glm::vec3& lightPosition);
	void EndDepthPass();

	void SendToShader(const Shader& shader) const;

	void Destroy();

private:
	GLuint m_FBO;
	GLuint m_depthTexture;
	GLsizei m_resolution;

	GLint m_previousViewport[4];

	glm::mat4 m_lightSpaceMatrix;
};
