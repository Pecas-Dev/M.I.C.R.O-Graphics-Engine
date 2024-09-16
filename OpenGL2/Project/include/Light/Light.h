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

	void MoveUp();
	void MoveDown();
	void MoveLeft();
	void MoveRight();
	void MoveForward();
	void MoveBackward();

	void MoveLight(Light& light);

	void Update() {}
	void Render(const Shader& shader);
	void SendToShader(const Shader& shader);
	void SetSpeed(GLfloat speed);
	
private:
	Buffer m_buffer;
	Transform m_transform;

	GLfloat m_speed;

	glm::mat4 m_model;
	
	glm::vec3 m_ambient;
	glm::vec3 m_diffuse;
	glm::vec3 m_specular;
};

