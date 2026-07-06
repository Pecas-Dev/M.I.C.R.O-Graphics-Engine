#pragma once

#include <Transform/Transform.h>
#include <Buffer/Buffer.h>
#include <Shader/Shader.h>

#include <glm.hpp>
#include <glad/gl.h>


class Light
{
public:
	Light();
	~Light();

	void Update() {}
	void Render(const Shader& shader);
	void SendToShader(const Shader& shader);

	Transform& GetTransform();

	glm::vec3& GetAmbient();
	glm::vec3& GetDiffuse();
	glm::vec3& GetSpecular();

	GLfloat& GetIntensity();
	GLfloat& GetLinearAttenuation();
	GLfloat& GetQuadraticAttenuation();

private:
	Buffer m_buffer;
	Transform m_transform;

	glm::mat4 m_model;

	glm::vec3 m_ambient;
	glm::vec3 m_diffuse;
	glm::vec3 m_specular;

	GLfloat m_intensity;
	GLfloat m_constantAttenuation;
	GLfloat m_linearAttenuation;
	GLfloat m_quadraticAttenuation;
};

