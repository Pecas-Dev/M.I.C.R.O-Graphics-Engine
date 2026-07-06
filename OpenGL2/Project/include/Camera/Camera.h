#pragma once

#include <Transform/Transform.h>
#include <Shader/Shader.h>

#include <glad/gl.h>
#include <glm.hpp>


class Camera
{
public:
	enum class Preset { Perspective, Front, Back, Left, Right, Top };

	Camera();

	void HandleNavigation(GLfloat deltaTime, bool viewportHovered);

	bool IsNavigating() const;

	void Set3DView(GLfloat fovDegrees, GLfloat aspectRatio);
	void SetViewPort(GLint x, GLint y, GLsizei width, GLsizei height);
	void SendToShader(const Shader& shader);

	const glm::mat4& GetViewMatrix() const;
	const glm::mat4& GetProjectionMatrix() const;

	void FlyTo(const glm::vec3& position, GLfloat pitch, GLfloat yaw);
	void FlyToLookAt(const glm::vec3& position, const glm::vec3& target);
	void ApplyPreset(Preset preset);
	void FrameModel();

	void UpdateMotion(GLfloat deltaTime);
	bool IsFlying() const;

	Transform& GetTransform();

protected:
	glm::mat4 m_view;
	glm::mat4 m_proj;

	glm::vec3 m_direction;
	glm::vec3 m_up;

private:
	enum class DragMode { None, Look, Orbit, Pan };

	glm::vec3 ForwardVector() const;

	void SnapToOrbit();

	void BeginDrag(DragMode mode);
	void EndDrag();

	Transform m_transform;

	glm::vec3 m_focus = glm::vec3(0.0f, 0.9f, 0.0f);
	GLfloat m_distance = 8.0f;

	GLfloat m_flySpeed = 4.0f;

	DragMode m_dragMode = DragMode::None;

	bool m_isFlying = false;
	glm::vec3 m_targetPosition = glm::vec3(0.0f);
	GLfloat m_targetPitch = 0.0f;
	GLfloat m_targetYaw = -90.0f;
	glm::vec3 m_targetFocus = glm::vec3(0.0f, 0.9f, 0.0f);
};
