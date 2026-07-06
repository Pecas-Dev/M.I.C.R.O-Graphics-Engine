#pragma once

#include <glad/gl.h>


class RenderTarget
{
public:
	bool Create(GLsizei width, GLsizei height, GLenum internalFormat = GL_RGBA8, bool hasDepth = true);

	void Resize(GLsizei width, GLsizei height);

	void Bind() const;
	void Unbind() const;

	GLuint GetColorTexture() const;

	GLsizei GetWidth() const;
	GLsizei GetHeight() const;

	void Destroy();

private:
	GLuint m_FBO = 0;
	GLuint m_colorTexture = 0;
	GLuint m_depthBuffer = 0;

	GLsizei m_width = 0;
	GLsizei m_height = 0;

	GLenum m_internalFormat = GL_RGBA8;
	bool m_hasDepth = true;
};
