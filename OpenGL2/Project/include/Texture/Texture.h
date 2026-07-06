#pragma once

#include <glad/gl.h>

#include <string>


class Texture
{
public:
	Texture();

	void Bind() const;
	void Bind(GLuint unit) const;
	GLuint GetID() const { return m_ID; }
	bool Load(const std::string& filename);
	bool LoadFromMemory(const void* data, int size);
	void Unbind() const;
	void Unload() const;

private:
	bool CreateFromSurface(struct SDL_Surface* textureData);

private:
	GLuint m_ID;
};

