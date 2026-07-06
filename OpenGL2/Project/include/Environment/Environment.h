#pragma once

#include <Shader/Shader.h>

#include <glad/gl.h>
#include <glm.hpp>

#include <string>


class Environment
{
public:
	bool Load(const std::string& hdrFilePath);

	void RenderSkybox(const glm::mat4& view, const glm::mat4& proj);

	void SendToShader(const Shader& shader) const;

	bool IsLoaded() const;

	const std::string& GetCurrentPath() const;

	bool& UseImageLighting();
	bool& ShowSkybox();
	float& GetIntensity();

	void Destroy();

private:
	void RenderUnitCube();

	GLuint m_environmentMap = 0;
	GLuint m_irradianceMap = 0;
	GLuint m_prefilterMap = 0;
	GLuint m_brdfLUT = 0;

	GLuint m_cubeVAO = 0;
	GLuint m_cubeVBO = 0;
	GLuint m_emptyVAO = 0;

	Shader m_skyboxShader;

	std::string m_currentPath;

	bool m_isLoaded = false;
	bool m_useImageLighting = true;
	bool m_showSkybox = true;
	float m_intensity = 1.0f;
};
