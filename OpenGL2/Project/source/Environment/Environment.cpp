#include <Environment/Environment.h>
#include <Utility/Utility.h>

#define STB_IMAGE_IMPLEMENTATION
#define STBI_ONLY_HDR
#include <vendor/stb/stb_image.h>

#include <gtc/matrix_transform.hpp>


namespace
{
	const GLsizei kEnvironmentSize = 512;
	const GLsizei kIrradianceSize = 32;
	const GLsizei kPrefilterSize = 128;
	const GLsizei kBrdfLutSize = 512;
	const GLint kPrefilterMipLevels = 5;

	const glm::mat4 kCaptureProjection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);

	const glm::mat4 kCaptureViews[6] =
	{
		glm::lookAt(glm::vec3(0.0f), glm::vec3( 1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
		glm::lookAt(glm::vec3(0.0f), glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
		glm::lookAt(glm::vec3(0.0f), glm::vec3( 0.0f,  1.0f,  0.0f), glm::vec3(0.0f,  0.0f,  1.0f)),
		glm::lookAt(glm::vec3(0.0f), glm::vec3( 0.0f, -1.0f,  0.0f), glm::vec3(0.0f,  0.0f, -1.0f)),
		glm::lookAt(glm::vec3(0.0f), glm::vec3( 0.0f,  0.0f,  1.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
		glm::lookAt(glm::vec3(0.0f), glm::vec3( 0.0f,  0.0f, -1.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
	};

	GLuint CreateCubemap(GLsizei size, bool mipmaps)
	{
		GLuint cubemap = 0;

		glGenTextures(1, &cubemap);
		glBindTexture(GL_TEXTURE_CUBE_MAP, cubemap);

		for (int face = 0; face < 6; face++)
		{
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, 0, GL_RGB16F, size, size, 0, GL_RGB, GL_FLOAT, nullptr);
		}

		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, mipmaps ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		if (mipmaps)
		{
			glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
		}

		return cubemap;
	}
}


bool Environment::Load(const std::string& hdrFilePath)
{
	stbi_set_flip_vertically_on_load(true);

	int width = 0;
	int height = 0;
	int components = 0;

	float* pixels = stbi_loadf(hdrFilePath.c_str(), &width, &height, &components, 3);

	if (!pixels)
	{
		Utility::AddMessage("Failed to load environment HDR: " + hdrFilePath);
		return false;
	}

	if (m_isLoaded)
	{
		Destroy();
	}

	GLuint equirectTexture = 0;
	glGenTextures(1, &equirectTexture);
	glBindTexture(GL_TEXTURE_2D, equirectTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, pixels);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	stbi_image_free(pixels);

	GLuint captureFBO = 0;
	glGenFramebuffers(1, &captureFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);

	GLint previousViewport[4];
	glGetIntegerv(GL_VIEWPORT, previousViewport);

	Shader captureShader;
	glDepthFunc(GL_LEQUAL);

	m_environmentMap = CreateCubemap(kEnvironmentSize, false);

	captureShader.Create("Assets/Shaders/CubeCapture.vert", "Assets/Shaders/EquirectToCube.frag");
	captureShader.UseShader();
	captureShader.SendUniformData("equirectangularMap", 0);
	captureShader.SendUniformData("proj", kCaptureProjection);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, equirectTexture);
	glViewport(0, 0, kEnvironmentSize, kEnvironmentSize);

	for (int face = 0; face < 6; face++)
	{
		captureShader.SendUniformData("view", kCaptureViews[face]);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, m_environmentMap, 0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		RenderUnitCube();
	}

	captureShader.Destroy();

	glBindTexture(GL_TEXTURE_CUBE_MAP, m_environmentMap);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

	glDeleteTextures(1, &equirectTexture);

	m_irradianceMap = CreateCubemap(kIrradianceSize, false);

	Shader irradianceShader;
	irradianceShader.Create("Assets/Shaders/CubeCapture.vert", "Assets/Shaders/Irradiance.frag");
	irradianceShader.UseShader();
	irradianceShader.SendUniformData("environmentMap", 0);
	irradianceShader.SendUniformData("proj", kCaptureProjection);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, m_environmentMap);
	glViewport(0, 0, kIrradianceSize, kIrradianceSize);

	for (int face = 0; face < 6; face++)
	{
		irradianceShader.SendUniformData("view", kCaptureViews[face]);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, m_irradianceMap, 0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		RenderUnitCube();
	}

	irradianceShader.Destroy();

	m_prefilterMap = CreateCubemap(kPrefilterSize, true);

	Shader prefilterShader;
	prefilterShader.Create("Assets/Shaders/CubeCapture.vert", "Assets/Shaders/Prefilter.frag");
	prefilterShader.UseShader();
	prefilterShader.SendUniformData("environmentMap", 0);
	prefilterShader.SendUniformData("proj", kCaptureProjection);
	prefilterShader.SendUniformData("sourceResolution", (GLfloat)kEnvironmentSize);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, m_environmentMap);

	for (GLint mip = 0; mip < kPrefilterMipLevels; mip++)
	{
		GLsizei mipSize = kPrefilterSize >> mip;
		glViewport(0, 0, mipSize, mipSize);

		float roughness = (float)mip / (float)(kPrefilterMipLevels - 1);
		prefilterShader.SendUniformData("roughness", roughness);

		for (int face = 0; face < 6; face++)
		{
			prefilterShader.SendUniformData("view", kCaptureViews[face]);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, m_prefilterMap, mip);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			RenderUnitCube();
		}
	}

	prefilterShader.Destroy();

	glGenTextures(1, &m_brdfLUT);
	glBindTexture(GL_TEXTURE_2D, m_brdfLUT);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RG16F, kBrdfLutSize, kBrdfLutSize, 0, GL_RG, GL_FLOAT, nullptr);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	Shader brdfShader;
	brdfShader.Create("Assets/Shaders/BRDF.vert", "Assets/Shaders/BRDF.frag");
	brdfShader.UseShader();

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_brdfLUT, 0);
	glViewport(0, 0, kBrdfLutSize, kBrdfLutSize);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	if (!m_emptyVAO) { glGenVertexArrays(1, &m_emptyVAO); }

	glBindVertexArray(m_emptyVAO);
	glDrawArrays(GL_TRIANGLES, 0, 3);
	glBindVertexArray(0);

	brdfShader.Destroy();

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glDeleteFramebuffers(1, &captureFBO);
	glViewport(previousViewport[0], previousViewport[1], previousViewport[2], previousViewport[3]);
	glDepthFunc(GL_LESS);

	m_skyboxShader.Create("Assets/Shaders/Skybox.vert", "Assets/Shaders/Skybox.frag");

	m_isLoaded = true;
	m_currentPath = hdrFilePath;

	size_t lastSlash = hdrFilePath.find_last_of("/\\");
	std::string fileName = (lastSlash == std::string::npos) ? hdrFilePath : hdrFilePath.substr(lastSlash + 1);
	Utility::AddMessage("Environment loaded: " + fileName + " (IBL ready)");

	return true;
}

void Environment::RenderUnitCube()
{
	if (!m_cubeVAO)
	{
		const GLfloat vertices[] =
		{
			-1.0f, -1.0f, -1.0f,  1.0f,  1.0f, -1.0f,  1.0f, -1.0f, -1.0f,
			 1.0f,  1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f,  1.0f, -1.0f,
			-1.0f, -1.0f,  1.0f,  1.0f, -1.0f,  1.0f,  1.0f,  1.0f,  1.0f,
			 1.0f,  1.0f,  1.0f, -1.0f,  1.0f,  1.0f, -1.0f, -1.0f,  1.0f,
			-1.0f,  1.0f,  1.0f, -1.0f,  1.0f, -1.0f, -1.0f, -1.0f, -1.0f,
			-1.0f, -1.0f, -1.0f, -1.0f, -1.0f,  1.0f, -1.0f,  1.0f,  1.0f,
			 1.0f,  1.0f,  1.0f,  1.0f, -1.0f, -1.0f,  1.0f,  1.0f, -1.0f,
			 1.0f, -1.0f, -1.0f,  1.0f,  1.0f,  1.0f,  1.0f, -1.0f,  1.0f,
			-1.0f, -1.0f, -1.0f,  1.0f, -1.0f, -1.0f,  1.0f, -1.0f,  1.0f,
			 1.0f, -1.0f,  1.0f, -1.0f, -1.0f,  1.0f, -1.0f, -1.0f, -1.0f,
			-1.0f,  1.0f, -1.0f,  1.0f,  1.0f,  1.0f,  1.0f,  1.0f, -1.0f,
			 1.0f,  1.0f,  1.0f, -1.0f,  1.0f, -1.0f, -1.0f,  1.0f,  1.0f,
		};

		glGenVertexArrays(1, &m_cubeVAO);
		glGenBuffers(1, &m_cubeVBO);

		glBindVertexArray(m_cubeVAO);
		glBindBuffer(GL_ARRAY_BUFFER, m_cubeVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (void*)0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
	}

	glBindVertexArray(m_cubeVAO);
	glDrawArrays(GL_TRIANGLES, 0, 36);
	glBindVertexArray(0);
}

void Environment::RenderSkybox(const glm::mat4& view, const glm::mat4& proj)
{
	if (!m_isLoaded || !m_showSkybox) { return; }

	glDepthFunc(GL_LEQUAL);

	m_skyboxShader.UseShader();
	m_skyboxShader.SendUniformData("view", view);
	m_skyboxShader.SendUniformData("proj", proj);
	m_skyboxShader.SendUniformData("environmentMap", 0);
	m_skyboxShader.SendUniformData("skyboxIntensity", m_intensity);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, m_environmentMap);

	RenderUnitCube();

	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
	glDepthFunc(GL_LESS);
}

void Environment::SendToShader(const Shader& shader) const
{
	bool active = m_isLoaded && m_useImageLighting;

	shader.SendUniformData("hasIBL", active ? 1 : 0);
	shader.SendUniformData("iblIntensity", m_intensity);

	shader.SendUniformData("irradianceMap", 5);
	shader.SendUniformData("prefilterMap", 6);
	shader.SendUniformData("brdfLUT", 7);

	if (!active) { return; }

	glActiveTexture(GL_TEXTURE5);
	glBindTexture(GL_TEXTURE_CUBE_MAP, m_irradianceMap);

	glActiveTexture(GL_TEXTURE6);
	glBindTexture(GL_TEXTURE_CUBE_MAP, m_prefilterMap);

	glActiveTexture(GL_TEXTURE7);
	glBindTexture(GL_TEXTURE_2D, m_brdfLUT);

	glActiveTexture(GL_TEXTURE0);
}

bool Environment::IsLoaded() const
{
	return m_isLoaded;
}

const std::string& Environment::GetCurrentPath() const
{
	return m_currentPath;
}

bool& Environment::UseImageLighting()
{
	return m_useImageLighting;
}

bool& Environment::ShowSkybox()
{
	return m_showSkybox;
}

float& Environment::GetIntensity()
{
	return m_intensity;
}

void Environment::Destroy()
{
	if (m_environmentMap) { glDeleteTextures(1, &m_environmentMap); m_environmentMap = 0; }
	if (m_irradianceMap) { glDeleteTextures(1, &m_irradianceMap); m_irradianceMap = 0; }
	if (m_prefilterMap) { glDeleteTextures(1, &m_prefilterMap); m_prefilterMap = 0; }
	if (m_brdfLUT) { glDeleteTextures(1, &m_brdfLUT); m_brdfLUT = 0; }
	if (m_cubeVAO) { glDeleteVertexArrays(1, &m_cubeVAO); m_cubeVAO = 0; }
	if (m_cubeVBO) { glDeleteBuffers(1, &m_cubeVBO); m_cubeVBO = 0; }
	if (m_emptyVAO) { glDeleteVertexArrays(1, &m_emptyVAO); m_emptyVAO = 0; }

	m_skyboxShader.Destroy();
	m_isLoaded = false;
}
