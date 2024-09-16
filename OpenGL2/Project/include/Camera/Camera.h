#pragma once

#include <Transform/Transform.h>
#include <Shader/Shader.h>

#include <glad/gl.h>
#include <glm.hpp>


class Camera
{
public:
	Camera();

	void MoveUp();
	void MoveDown();
	void MoveLeft();
	void MoveRight();
	void MoveForward();
	void MoveBackward();
	void RotateRight();
	void RotateLeft();

	void ZoomCamera(Camera& camera);
	void MoveCamera(Camera& camera);
	void RotateCamera(Camera& camera);

	void Set3DView();
	void SetViewPort(GLint x, GLint y, GLsizei width, GLsizei height);
	void Update() {}
	void SendToShader(const Shader& shader);
	void SetMoveSpeed(GLfloat moveSpeed);
	void SetZoomSpeed(GLfloat zoomSpeed);
	void SetRotationSpeed(GLfloat rotationSpeed);

protected:
	glm::mat4 m_view;
	glm::mat4 m_proj;

	glm::vec3 m_direction;
	glm::vec3 m_up;

private:
	Transform m_transform;

	GLfloat m_moveSpeed;
	GLfloat m_zoomSpeed;
	GLfloat m_rotationSpeed;
};
