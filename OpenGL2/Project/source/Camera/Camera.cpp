#include <InputSystem/Input.h>
#include <Camera/Camera.h>
#include <Shader/Shader.h>

#include <gtc\matrix_transform.hpp>

#include <iostream>


Camera::Camera()
{
	m_view = glm::mat4(1.0f);
	m_proj = glm::mat4(1.0f);
	m_transform.SetPosition(0.0f, 1.0f, 25.0f);
	m_transform.SetRotation(0.0f, -90.0f, 0.0f);

	m_direction = glm::vec3(0.0f, 0.0f, -1.0f);
	m_up = glm::vec3(0.0f, 1.0f, 0.0f);
}

void Camera::MoveUp()
{
	auto position = m_transform.GetPosition();
	position.y += m_moveSpeed;
	m_transform.SetPosition(position.x, position.y, position.z);
}

void Camera::MoveDown()
{
	auto position = m_transform.GetPosition();
	position.y -= m_moveSpeed;
	m_transform.SetPosition(position.x, position.y, position.z);
}

void Camera::MoveLeft()
{
	auto position = m_transform.GetPosition();
	position.x -= m_moveSpeed;
	m_transform.SetPosition(position.x, position.y, position.z);
}

void Camera::MoveRight()
{
	auto position = m_transform.GetPosition();
	position.x += m_moveSpeed;
	m_transform.SetPosition(position.x, position.y, position.z);
}

void Camera::MoveForward()
{
	auto position = m_transform.GetPosition();
	glm::vec3 forward = glm::normalize(m_direction);
	position += forward * m_zoomSpeed;
	m_transform.SetPosition(position.x, position.y, position.z);
}

void Camera::MoveBackward()
{
	auto position = m_transform.GetPosition();
	glm::vec3 backward = -glm::normalize(m_direction);
	position += backward * m_zoomSpeed;
	m_transform.SetPosition(position.x, position.y, position.z);
}

void Camera::RotateRight()
{
	auto rotation = m_transform.GetRotation();
	rotation.y += m_rotationSpeed;
	m_transform.SetRotation(rotation.x, rotation.y, rotation.z);
}

void Camera::RotateLeft()
{
	auto rotation = m_transform.GetRotation();
	rotation.y -= m_rotationSpeed;
	m_transform.SetRotation(rotation.x, rotation.y, rotation.z);
}

void Camera::ZoomCamera(Camera& camera)
{
	if (Input::Instance()->GetMouseWheel() > 0)
	{
		camera.MoveForward();
	}
	else if (Input::Instance()->GetMouseWheel() < 0)
	{
		camera.MoveBackward();
	}
}

void Camera::MoveCamera(Camera& camera)
{
	if (Input::Instance()->IsLeftButtonClicked())
	{
		if (Input::Instance()->GetMouseMotionY() > 0)
		{
			camera.MoveDown();
		}
		else if (Input::Instance()->GetMouseMotionY() < 0)
		{
			camera.MoveUp();
		}
		else if (Input::Instance()->GetMouseMotionX() > 0)
		{
			camera.MoveRight();
		}
		else if (Input::Instance()->GetMouseMotionX() < 0)
		{
			camera.MoveLeft();
		}
	}
}

void Camera::RotateCamera(Camera& camera)
{
	if (Input::Instance()->IsRightButtonClicked())
	{
		if (Input::Instance()->GetMouseMotionX() > 0)
		{
			camera.RotateRight();
		}
		else if (Input::Instance()->GetMouseMotionX() < 0)
		{
			camera.RotateLeft();
		}
	}
}

void Camera::Set3DView()
{
	GLfloat FOV = 45.0f;
	GLfloat aspectRatio = 1280.0f / 720.0f;

	m_proj = glm::perspective(FOV, aspectRatio, 0.001f, 1000.0f);
}

void Camera::SetViewPort(GLint x, GLint y, GLsizei width, GLsizei height)
{
	glViewport(x, y, width, height);
}

void Camera::SendToShader(const Shader& shader)
{
	auto position = m_transform.GetPosition();
	auto rotation = m_transform.GetRotation();

	m_direction.x = cos(glm::radians(rotation.x)) * cos(glm::radians(rotation.y));
	m_direction.y = sin(glm::radians(rotation.x));
	m_direction.z = cos(glm::radians(rotation.x)) * sin(glm::radians(rotation.y));
	m_direction = glm::normalize(m_direction);

	m_view = glm::lookAt(position, position + m_direction, m_up);

	shader.SendUniformData("proj", m_proj);
	shader.SendUniformData("view", m_view);
	shader.SendUniformData("cameraPosition", position.x, position.y, position.z);
}

void Camera::SetMoveSpeed(GLfloat moveSpeed)
{
	m_moveSpeed = moveSpeed;
}

void Camera::SetZoomSpeed(GLfloat zoomSpeed)
{
	m_zoomSpeed = zoomSpeed;
}

void Camera::SetRotationSpeed(GLfloat rotationSpeed)
{
	m_rotationSpeed = rotationSpeed;
}

