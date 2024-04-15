#include "Camera.h"
#include "Configs.h"

// WARNING Need to set GLM_FORCE_DEPTH_ZERO_TO_ONE in the vcxproj config
#include "glm/glm.hpp"
#include "glm/ext.hpp"

#include <iostream>

Camera::Camera(
	glm::vec3 position,
	glm::vec3 worldUp,
	float screenWidth,
	float screenHeight,
	float yaw,
	float pitch) :
	position_(position),
	worldUp_(worldUp),
	yaw_(yaw),
	pitch_(pitch),
	front_(glm::vec3(0.f)),
	movementSpeed_(CameraConfig::Speed),
	mouseSensitivity_(CameraConfig::Sensitivity),
	zoom_(CameraConfig::Zoom),
	screenWidth_(screenWidth),
	screenHeight_(screenHeight)
{
	// Update orthogonal axes and matrices
	UpdateInternal();
}

void Camera::SetScreenSize(float width, float height)
{
	screenWidth_ = width;
	screenHeight_ = height;

	// Update orthogonal axes and matrices
	UpdateInternal();
}

void Camera::ProcessKeyboard(CameraMovement direction, float deltaTime_)
{
	const float velocity = movementSpeed_ * deltaTime_;
	if (direction == CameraMovement::Forward)
	{
		position_ += front_ * velocity;
	}
	else if (direction == CameraMovement::Backward)
	{
		position_ -= front_ * velocity;
	}
	else if (direction == CameraMovement::Left)
	{
		position_ -= right_ * velocity;
	}
	else if (direction == CameraMovement::Right)
	{
		position_ += right_ * velocity;
	}

	// Update orthogonal axes and matrices
	UpdateInternal();
}

// Processes input received from a mouse input system. 
// Expects the offset value in both the x and y direction.
void Camera::ProcessMouseMovement(float xoffset, float yoffset)
{
	xoffset *= mouseSensitivity_;
	yoffset *= mouseSensitivity_;

	yaw_ += xoffset;
	pitch_ += yoffset;

	ConstrainPitch();

	// Update orthogonal axes and matrices
	UpdateInternal();
}

void Camera::SetPositionAndTarget(glm::vec3 cameraPosition, glm::vec3 cameraTarget)
{
	position_ = cameraPosition;

	const glm::vec3 direction = glm::normalize(cameraTarget - position_);
	pitch_ = asin(direction.y);
	yaw_ = atan2(direction.z, direction.x);

	// TODO Inefficient because keep converting back-and-forth between degrees and radians
	pitch_ = glm::degrees(pitch_);
	yaw_ = glm::degrees(yaw_);

	ConstrainPitch();

	// Update orthogonal axes and matrices
	UpdateInternal();
}

void Camera::ConstrainPitch()
{
	// Make sure that when pitch is out of bounds, screen doesn't get flipped
	constexpr float pitchLimit = 89.f;
	pitch_ = glm::clamp(pitch_, -pitchLimit, pitchLimit);
}

// Processes input received from a mouse scroll-wheel event. 
// Only requires input on the vertical wheel-axis
void Camera::ProcessMouseScroll(float yoffset)
{
	zoom_ -= (float)yoffset;
	zoom_ = glm::clamp(zoom_, 1.f, 45.f);

	// Update orthogonal axes and matrices
	UpdateInternal();
}

void Camera::UpdateInternal()
{
	// Calculate the new Front vector
	glm::vec3 front;
	front.x = cos(glm::radians(yaw_)) * cos(glm::radians(pitch_));
	front.y = sin(glm::radians(pitch_));
	front.z = sin(glm::radians(yaw_)) * cos(glm::radians(pitch_));
	front_ = glm::normalize(front);

	// Calculate three orthogonal axes
	right_ = glm::normalize(glm::cross(front_, worldUp_));
	up_ = glm::normalize(glm::cross(right_, front_));

	// Projection matrix
	projectionMatrix_ = glm::perspective(glm::radians(zoom_),
		static_cast<float>(screenWidth_) / static_cast<float>(screenHeight_),
		CameraConfig::Near,
		CameraConfig::Far);
	inverseProjectionMatrix_ = glm::inverse(projectionMatrix_);

	// View matrix
	viewMatrix_ = glm::lookAt(position_, position_ + front_, up_);
}

glm::mat4 Camera::GetProjectionMatrix() const
{
	return projectionMatrix_;
}

glm::mat4 Camera::GetInverseProjectionMatrix() const
{
	return inverseProjectionMatrix_;
}

glm::mat4 Camera::GetViewMatrix() const
{
	return viewMatrix_;
}

glm::mat4 Camera::GetInverseViewMatrix() const
{
	return glm::inverse(viewMatrix_);
}

glm::vec3 Camera::Position() const
{
	return position_;
}