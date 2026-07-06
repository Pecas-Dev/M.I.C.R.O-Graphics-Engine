#include <Texture/Texture.h>
#include <Utility/Utility.h>

#include <SDL_image.h>

#include <cstring>
#include <iostream>
#include <vector>


Texture::Texture()
{
	m_ID = 0;
}

void Texture::Bind() const
{
	glBindTexture(GL_TEXTURE_2D, m_ID);
}

void Texture::Bind(GLuint unit) const
{
	glActiveTexture(GL_TEXTURE0 + unit);
	glBindTexture(GL_TEXTURE_2D, m_ID);
	glActiveTexture(GL_TEXTURE0);
}

bool Texture::Load(const std::string& filename)
{
	SDL_Surface* textureData = IMG_Load(filename.c_str());

	if (!textureData)
	{
		Utility::AddMessage("Error loading texture: " + filename);
		return false;
	}

	return CreateFromSurface(textureData);
}

bool Texture::LoadFromMemory(const void* data, int size)
{
	SDL_Surface* textureData = IMG_Load_RW(SDL_RWFromConstMem(data, size), 1);

	if (!textureData)
	{
		Utility::AddMessage("Error loading embedded texture!");
		return false;
	}

	return CreateFromSurface(textureData);
}

bool Texture::CreateFromSurface(SDL_Surface* textureData)
{
	SDL_Surface* converted = SDL_ConvertSurfaceFormat(textureData, SDL_PIXELFORMAT_RGBA32, 0);
	SDL_FreeSurface(textureData);

	if (!converted)
	{
		Utility::AddMessage("Error converting texture format!");
		return false;
	}

	textureData = converted;

	GLsizei width = textureData->w;
	GLsizei height = textureData->h;
	Uint8* pixels = (Uint8*)textureData->pixels;
	GLint format = GL_RGBA;

	const int pitch = textureData->pitch;
	std::vector<Uint8> rowBuffer(pitch);

	for (int row = 0; row < height / 2; row++)
	{
		Uint8* top = pixels + static_cast<size_t>(row) * pitch;
		Uint8* bottom = pixels + static_cast<size_t>(height - 1 - row) * pitch;

		std::memcpy(rowBuffer.data(), top, pitch);
		std::memcpy(top, bottom, pitch);
		std::memcpy(bottom, rowBuffer.data(), pitch);
	}

	glGenTextures(1, &m_ID);

	glBindTexture(GL_TEXTURE_2D, m_ID);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, pixels);
		glGenerateMipmap(GL_TEXTURE_2D);

	glBindTexture(GL_TEXTURE_2D, 0);

	SDL_FreeSurface(textureData);

	return true;
}

void Texture::Unbind() const
{
	glBindTexture(GL_TEXTURE_2D, 0);
}

void Texture::Unload() const
{
	glDeleteTextures(1, &m_ID);
}