#include <RenderTarget/RenderTarget.h>
#include <Utility/Utility.h>


bool RenderTarget::Create(GLsizei width, GLsizei height, GLenum internalFormat, bool hasDepth)
{
	m_width = (width > 0) ? width : 1;
	m_height = (height > 0) ? height : 1;
	m_internalFormat = internalFormat;
	m_hasDepth = hasDepth;

	GLenum dataType = (internalFormat == GL_RGBA16F || internalFormat == GL_RGBA32F) ? GL_FLOAT : GL_UNSIGNED_BYTE;

	glGenTextures(1, &m_colorTexture);
	glBindTexture(GL_TEXTURE_2D, m_colorTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, m_width, m_height, 0, GL_RGBA, dataType, nullptr);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glGenFramebuffers(1, &m_FBO);
	glBindFramebuffer(GL_FRAMEBUFFER, m_FBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_colorTexture, 0);

	if (hasDepth)
	{
		glGenRenderbuffers(1, &m_depthBuffer);
		glBindRenderbuffer(GL_RENDERBUFFER, m_depthBuffer);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, m_width, m_height);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_depthBuffer);
	}

	bool isComplete = (glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glBindTexture(GL_TEXTURE_2D, 0);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);

	if (!isComplete)
	{
		Utility::AddMessage("Failed to create the scene render target!");
	}

	return isComplete;
}

void RenderTarget::Resize(GLsizei width, GLsizei height)
{
	if (width <= 0 || height <= 0) { return; }
	if (width == m_width && height == m_height) { return; }

	GLenum internalFormat = m_internalFormat;
	bool hasDepth = m_hasDepth;

	Destroy();
	Create(width, height, internalFormat, hasDepth);
}

void RenderTarget::Bind() const
{
	glBindFramebuffer(GL_FRAMEBUFFER, m_FBO);
	glViewport(0, 0, m_width, m_height);
}

void RenderTarget::Unbind() const
{
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

GLuint RenderTarget::GetColorTexture() const
{
	return m_colorTexture;
}

GLsizei RenderTarget::GetWidth() const
{
	return m_width;
}

GLsizei RenderTarget::GetHeight() const
{
	return m_height;
}

void RenderTarget::Destroy()
{
	if (m_FBO) { glDeleteFramebuffers(1, &m_FBO); m_FBO = 0; }
	if (m_colorTexture) { glDeleteTextures(1, &m_colorTexture); m_colorTexture = 0; }
	if (m_depthBuffer) { glDeleteRenderbuffers(1, &m_depthBuffer); m_depthBuffer = 0; }
}
