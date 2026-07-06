#pragma once

#include <RenderTarget/RenderTarget.h>
#include <Shader/Shader.h>

#include <glad/gl.h>


class PostProcess
{
public:
	bool Create(GLsizei width, GLsizei height);
	void Resize(GLsizei width, GLsizei height);

	GLuint Apply(GLuint hdrSceneTexture);

	const RenderTarget& GetOutput() const;

	float& GetExposure();
	bool& BloomEnabled();
	float& GetBloomStrength();
	float& GetBloomThreshold();

	void Destroy();

private:
	void DrawFullscreen();

	RenderTarget m_brightTarget;   
	RenderTarget m_blurTargets[2]; 
	RenderTarget m_outputTarget;   

	Shader m_brightShader;
	Shader m_blurShader;
	Shader m_tonemapShader;

	GLuint m_emptyVAO = 0;

	float m_exposure = 1.0f;
	bool m_bloomEnabled = true;
	float m_bloomStrength = 0.6f;
	float m_bloomThreshold = 1.1f;
};
