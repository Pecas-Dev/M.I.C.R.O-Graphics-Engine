#include <Shadow/ShadowMap.h>
#include <Utility/Utility.h>

#include <gtc/matrix_transform.hpp>

#include <cmath>


ShadowMap::ShadowMap()
{
	m_FBO = 0;
	m_depthTexture = 0;
	m_resolution = 0;
	m_previousViewport[0] = m_previousViewport[1] = m_previousViewport[2] = m_previousViewport[3] = 0;
	m_lightSpaceMatrix = glm::mat4(1.0f);
}

bool ShadowMap::Create(GLsizei resolution)
{
	m_resolution = resolution;

	glGenTextures(1, &m_depthTexture);
	glBindTexture(GL_TEXTURE_2D, m_depthTexture);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, resolution, resolution, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	const GLfloat borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

	glGenFramebuffers(1, &m_FBO);
	glBindFramebuffer(GL_FRAMEBUFFER, m_FBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_depthTexture, 0);

	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);

	bool isComplete = (glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glBindTexture(GL_TEXTURE_2D, 0);

	if (!isComplete)
	{
		Utility::AddMessage("Failed to create the shadow map framebuffer!");
	}

	return isComplete;
}

void ShadowMap::BeginDepthPass(const glm::vec3& lightPosition)
{
	glm::vec3 direction = glm::normalize(-lightPosition);
	glm::vec3 up = (std::fabs(direction.y) > 0.99f) ? glm::vec3(0.0f, 0.0f, -1.0f) : glm::vec3(0.0f, 1.0f, 0.0f);

	glm::mat4 lightView = glm::lookAt(lightPosition, glm::vec3(0.0f), up);
	glm::mat4 lightProjection = glm::perspective(glm::radians(100.0f), 1.0f, 0.5f, 60.0f);

	m_lightSpaceMatrix = lightProjection * lightView;

	glGetIntegerv(GL_VIEWPORT, m_previousViewport);

	glViewport(0, 0, m_resolution, m_resolution);
	glBindFramebuffer(GL_FRAMEBUFFER, m_FBO);
	glClear(GL_DEPTH_BUFFER_BIT);
}

void ShadowMap::EndDepthPass()
{
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(m_previousViewport[0], m_previousViewport[1], m_previousViewport[2], m_previousViewport[3]);
}

void ShadowMap::SendToShader(const Shader& shader) const
{
	shader.SendUniformData("lightSpaceMatrix", m_lightSpaceMatrix);
	shader.SendUniformData("hasShadows", true);
	shader.SendUniformData("shadowMap", 4);

	glActiveTexture(GL_TEXTURE4);
	glBindTexture(GL_TEXTURE_2D, m_depthTexture);
	glActiveTexture(GL_TEXTURE0);
}

void ShadowMap::Destroy()
{
	glDeleteFramebuffers(1, &m_FBO);
	glDeleteTextures(1, &m_depthTexture);
}
