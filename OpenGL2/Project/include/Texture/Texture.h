#pragma once

#include <glad/gl.h>

#include <string>


class Texture
{
public:
	Texture();

	void Bind() const;
	bool Load(const std::string& filename);
	void Unbind() const;
	void Unload() const;

private:
	GLuint m_ID;
};

