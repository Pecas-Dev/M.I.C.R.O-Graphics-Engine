#include <InputSystem/Input.h>
#include <Camera/Camera.h>
#include <Shader/Shader.h>

#include <gtc\matrix_transform.hpp>

#include <cmath>
#include <algorithm>


namespace
{
	const GLfloat kLookSensitivity = 0.15f;   
	const GLfloat kOrbitSensitivity = 0.25f;  
	const GLfloat kPanScale = 0.0018f;        
	const GLfloat kDollyStep = 0.88f;         
	const GLfloat kFlyBoost = 3.5f;           
	const GLfloat kPitchLimit = 89.0f;

	void PitchYawFromDirection(const glm::vec3& direction, GLfloat& pitch, GLfloat& yaw)
	{
		pitch = glm::degrees(asinf(glm::clamp(direction.y, -1.0f, 1.0f)));
		yaw = glm::degrees(atan2f(direction.z, direction.x));
	}
}


Camera::Camera()
{
	m_view = glm::mat4(1.0f);
	m_proj = glm::mat4(1.0f);

	m_direction = glm::vec3(0.0f, 0.0f, -1.0f);
	m_up = glm::vec3(0.0f, 1.0f, 0.0f);

	glm::vec3 position(4.0f, 2.6f, 4.6f);

	m_transform.SetPosition(position.x, position.y, position.z);

	glm::vec3 direction = glm::normalize(m_focus - position);

	GLfloat pitch = 0.0f;
	GLfloat yaw = 0.0f;
	PitchYawFromDirection(direction, pitch, yaw);

	m_transform.SetRotation(pitch, yaw, 0.0f);

	m_distance = glm::length(m_focus - position);
	m_targetFocus = m_focus;
}

glm::vec3 Camera::ForwardVector() const
{
	auto rotation = m_transform.GetRotation();

	glm::vec3 forward;
	forward.x = cosf(glm::radians(rotation.x)) * cosf(glm::radians(rotation.y));
	forward.y = sinf(glm::radians(rotation.x));
	forward.z = cosf(glm::radians(rotation.x)) * sinf(glm::radians(rotation.y));

	return glm::normalize(forward);
}

void Camera::SnapToOrbit()
{
	glm::vec3 position = m_focus - ForwardVector() * m_distance;
	m_transform.SetPosition(position.x, position.y, position.z);
}

void Camera::BeginDrag(DragMode mode)
{
	m_dragMode = mode;
	m_isFlying = false;

	SDL_SetRelativeMouseMode(SDL_TRUE);
}

void Camera::EndDrag()
{
	m_dragMode = DragMode::None;
	SDL_SetRelativeMouseMode(SDL_FALSE);
}

bool Camera::IsNavigating() const
{
	return m_dragMode != DragMode::None;
}

void Camera::HandleNavigation(GLfloat deltaTime, bool viewportHovered)
{
	auto input = Input::Instance();

	if (m_dragMode == DragMode::None && viewportHovered)
	{
		if (input->WasRightButtonPressed()) { BeginDrag(DragMode::Look); }
		else if (input->WasMiddleButtonPressed()) { BeginDrag(DragMode::Pan); }
		else if (input->WasLeftButtonPressed()) { BeginDrag(DragMode::Orbit); }
	}

	if ((m_dragMode == DragMode::Look && !input->IsRightButtonClicked()) ||
		(m_dragMode == DragMode::Pan && !input->IsMiddleButtonClicked()) ||
		(m_dragMode == DragMode::Orbit && !input->IsLeftButtonClicked()))
	{
		EndDrag();
	}

	GLfloat dx = static_cast<GLfloat>(input->GetMouseMotionX());
	GLfloat dy = static_cast<GLfloat>(input->GetMouseMotionY());
	int wheel = input->GetMouseWheel();

	auto position = m_transform.GetPosition();
	auto rotation = m_transform.GetRotation(); 

	glm::vec3 forward = ForwardVector();
	glm::vec3 right = glm::normalize(glm::cross(forward, m_up));
	glm::vec3 up = glm::normalize(glm::cross(right, forward));

	switch (m_dragMode)
	{

	case DragMode::Look:
	{
		rotation.y += dx * kLookSensitivity;
		rotation.x -= dy * kLookSensitivity;
		rotation.x = glm::clamp(rotation.x, -kPitchLimit, kPitchLimit);

		m_transform.SetRotation(rotation.x, rotation.y, rotation.z);
		forward = ForwardVector();
		right = glm::normalize(glm::cross(forward, m_up));

		glm::vec3 move(0.0f);

		if (input->IsKeyHeld(SDL_SCANCODE_W)) { move += forward; }
		if (input->IsKeyHeld(SDL_SCANCODE_S)) { move -= forward; }
		if (input->IsKeyHeld(SDL_SCANCODE_D)) { move += right; }
		if (input->IsKeyHeld(SDL_SCANCODE_A)) { move -= right; }
		if (input->IsKeyHeld(SDL_SCANCODE_E)) { move += m_up; }
		if (input->IsKeyHeld(SDL_SCANCODE_Q)) { move -= m_up; }

		if (wheel != 0)
		{
			m_flySpeed = glm::clamp(m_flySpeed * powf(1.15f, static_cast<GLfloat>(wheel)), 0.4f, 40.0f);
		}

		if (glm::length(move) > 0.0f)
		{
			GLfloat speed = m_flySpeed * (input->IsShiftDown() ? kFlyBoost : 1.0f);
			position += glm::normalize(move) * speed * deltaTime;
			m_transform.SetPosition(position.x, position.y, position.z);
		}

		m_focus = position + forward * m_distance;
		break;
	}

	case DragMode::Orbit:
	{
		rotation.y += dx * kOrbitSensitivity;
		rotation.x += dy * kOrbitSensitivity;
		rotation.x = glm::clamp(rotation.x, -kPitchLimit, kPitchLimit);

		m_transform.SetRotation(rotation.x, rotation.y, rotation.z);
		SnapToOrbit();
		break;
	}

	case DragMode::Pan:
	{
		glm::vec3 offset = (-right * dx + up * dy) * (m_distance * kPanScale);

		m_focus += offset;
		position += offset;
		m_transform.SetPosition(position.x, position.y, position.z);
		break;
	}

	case DragMode::None:
	{
		break;
	}

	}

	if (m_dragMode != DragMode::Look && wheel != 0 && viewportHovered)
	{
		m_isFlying = false;
		m_distance = glm::clamp(m_distance * powf(kDollyStep, static_cast<GLfloat>(wheel)), 0.3f, 150.0f);
		SnapToOrbit();
	}
}

void Camera::Set3DView(GLfloat fovDegrees, GLfloat aspectRatio)
{
	m_proj = glm::perspective(glm::radians(fovDegrees), aspectRatio, 0.05f, 500.0f);
}

void Camera::SetViewPort(GLint x, GLint y, GLsizei width, GLsizei height)
{
	glViewport(x, y, width, height);
}

void Camera::SendToShader(const Shader& shader)
{
	auto position = m_transform.GetPosition();

	m_direction = ForwardVector();
	m_view = glm::lookAt(position, position + m_direction, m_up);

	shader.SendUniformData("proj", m_proj);
	shader.SendUniformData("view", m_view);
	shader.SendUniformData("cameraPosition", position.x, position.y, position.z);
}

const glm::mat4& Camera::GetViewMatrix() const
{
	return m_view;
}

const glm::mat4& Camera::GetProjectionMatrix() const
{
	return m_proj;
}

void Camera::FlyTo(const glm::vec3& position, GLfloat pitch, GLfloat yaw)
{
	m_targetPosition = position;
	m_targetPitch = pitch;
	m_targetYaw = yaw;
	m_isFlying = true;
}

void Camera::FlyToLookAt(const glm::vec3& position, const glm::vec3& target)
{
	glm::vec3 direction = glm::normalize(target - position);

	GLfloat pitch = 0.0f;
	GLfloat yaw = 0.0f;
	PitchYawFromDirection(direction, pitch, yaw);

	FlyTo(position, pitch, yaw);

	m_targetFocus = target;
}

void Camera::ApplyPreset(Preset preset)
{
	const glm::vec3 focus(0.0f, 0.9f, 0.0f);

	switch (preset)
	{
	case Preset::Perspective: { FlyToLookAt(glm::vec3(4.0f, 2.6f, 4.6f), focus); break; }
	case Preset::Front:       { FlyToLookAt(glm::vec3(0.0f, 1.1f, 6.2f), focus); break; }
	case Preset::Back:        { FlyToLookAt(glm::vec3(0.0f, 1.1f, -6.2f), focus); break; }
	case Preset::Left:        { FlyToLookAt(glm::vec3(-6.2f, 1.1f, 0.0f), focus); break; }
	case Preset::Right:       { FlyToLookAt(glm::vec3(6.2f, 1.1f, 0.0f), focus); break; }
	case Preset::Top:         { FlyToLookAt(glm::vec3(0.0f, 7.5f, 0.02f), glm::vec3(0.0f, 0.0f, 0.0f)); break; }
	}
}

void Camera::FrameModel()
{
	FlyToLookAt(glm::vec3(2.4f, 1.9f, 3.4f), glm::vec3(0.0f, 0.9f, 0.0f));
}

void Camera::UpdateMotion(GLfloat deltaTime)
{
	if (!m_isFlying) { return; }

	GLfloat t = 1.0f - expf(-8.0f * deltaTime);

	glm::vec3 position = glm::mix(m_transform.GetPosition(), m_targetPosition, t);

	m_focus = glm::mix(m_focus, m_targetFocus, t);

	auto rotation = m_transform.GetRotation();

	GLfloat yawDelta = m_targetYaw - rotation.y;
	while (yawDelta > 180.0f) { yawDelta -= 360.0f; }
	while (yawDelta < -180.0f) { yawDelta += 360.0f; }

	GLfloat pitch = rotation.x + (m_targetPitch - rotation.x) * t;
	GLfloat yaw = rotation.y + yawDelta * t;

	m_transform.SetPosition(position.x, position.y, position.z);
	m_transform.SetRotation(pitch, yaw, rotation.z);

	bool arrived = glm::length(m_targetPosition - position) < 0.005f &&
		fabsf(m_targetPitch - pitch) < 0.05f && fabsf(yawDelta) < 0.05f;

	if (arrived)
	{
		m_transform.SetPosition(m_targetPosition.x, m_targetPosition.y, m_targetPosition.z);
		m_transform.SetRotation(m_targetPitch, m_targetYaw, rotation.z);

		m_focus = m_targetFocus;
		m_distance = glm::length(m_targetPosition - m_targetFocus);

		m_isFlying = false;
	}
}

bool Camera::IsFlying() const
{
	return m_isFlying;
}

Transform& Camera::GetTransform()
{
	return m_transform;
}
