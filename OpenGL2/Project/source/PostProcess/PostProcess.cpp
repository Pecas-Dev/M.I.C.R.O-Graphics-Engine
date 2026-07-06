#include <PostProcess/PostProcess.h>


bool PostProcess::Create(GLsizei width, GLsizei height)
{
	GLsizei halfWidth = (width > 1) ? width / 2 : 1;
	GLsizei halfHeight = (height > 1) ? height / 2 : 1;

	bool ok = true;

	ok &= m_brightTarget.Create(halfWidth, halfHeight, GL_RGBA16F, false);
	ok &= m_blurTargets[0].Create(halfWidth, halfHeight, GL_RGBA16F, false);
	ok &= m_blurTargets[1].Create(halfWidth, halfHeight, GL_RGBA16F, false);
	ok &= m_outputTarget.Create(width, height, GL_RGBA8, false);

	ok &= m_brightShader.Create("Assets/Shaders/Post.vert", "Assets/Shaders/BrightPass.frag");
	ok &= m_blurShader.Create("Assets/Shaders/Post.vert", "Assets/Shaders/Blur.frag");
	ok &= m_tonemapShader.Create("Assets/Shaders/Post.vert", "Assets/Shaders/Tonemap.frag");

	glGenVertexArrays(1, &m_emptyVAO);

	return ok;
}

void PostProcess::Resize(GLsizei width, GLsizei height)
{
	GLsizei halfWidth = (width > 1) ? width / 2 : 1;
	GLsizei halfHeight = (height > 1) ? height / 2 : 1;

	m_brightTarget.Resize(halfWidth, halfHeight);
	m_blurTargets[0].Resize(halfWidth, halfHeight);
	m_blurTargets[1].Resize(halfWidth, halfHeight);
	m_outputTarget.Resize(width, height);
}

void PostProcess::DrawFullscreen()
{
	glBindVertexArray(m_emptyVAO);
	glDrawArrays(GL_TRIANGLES, 0, 3);
	glBindVertexArray(0);
}

GLuint PostProcess::Apply(GLuint hdrSceneTexture)
{
	glDisable(GL_DEPTH_TEST);

	GLuint bloomTexture = 0;

	if (m_bloomEnabled)
	{
		m_brightTarget.Bind();

		m_brightShader.UseShader();
		m_brightShader.SendUniformData("sceneTexture", 0);
		m_brightShader.SendUniformData("threshold", m_bloomThreshold);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, hdrSceneTexture);

		DrawFullscreen();

		m_blurShader.UseShader();
		m_blurShader.SendUniformData("sourceTexture", 0);

		GLuint source = m_brightTarget.GetColorTexture();
		int writeIndex = 0;

		const int blurPasses = 8;

		for (int pass = 0; pass < blurPasses; pass++)
		{
			m_blurTargets[writeIndex].Bind();
			m_blurShader.SendUniformData("horizontal", (pass % 2 == 0) ? 1 : 0);

			glBindTexture(GL_TEXTURE_2D, source);
			DrawFullscreen();

			source = m_blurTargets[writeIndex].GetColorTexture();
			writeIndex = 1 - writeIndex;
		}

		bloomTexture = source;
	}

	m_outputTarget.Bind();

	m_tonemapShader.UseShader();
	m_tonemapShader.SendUniformData("sceneTexture", 0);
	m_tonemapShader.SendUniformData("bloomTexture", 1);
	m_tonemapShader.SendUniformData("exposure", m_exposure);
	m_tonemapShader.SendUniformData("bloomStrength", m_bloomStrength);
	m_tonemapShader.SendUniformData("bloomEnabled", (m_bloomEnabled && bloomTexture) ? 1 : 0);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, hdrSceneTexture);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, bloomTexture ? bloomTexture : hdrSceneTexture);

	DrawFullscreen();

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, 0);

	m_outputTarget.Unbind();
	glEnable(GL_DEPTH_TEST);

	return m_outputTarget.GetColorTexture();
}

const RenderTarget& PostProcess::GetOutput() const
{
	return m_outputTarget;
}

float& PostProcess::GetExposure()
{
	return m_exposure;
}

bool& PostProcess::BloomEnabled()
{
	return m_bloomEnabled;
}

float& PostProcess::GetBloomStrength()
{
	return m_bloomStrength;
}

float& PostProcess::GetBloomThreshold()
{
	return m_bloomThreshold;
}

void PostProcess::Destroy()
{
	m_brightTarget.Destroy();
	m_blurTargets[0].Destroy();
	m_blurTargets[1].Destroy();
	m_outputTarget.Destroy();

	m_brightShader.Destroy();
	m_blurShader.Destroy();
	m_tonemapShader.Destroy();

	if (m_emptyVAO) { glDeleteVertexArrays(1, &m_emptyVAO); m_emptyVAO = 0; }
}
