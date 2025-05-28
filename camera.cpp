#include "camera.hpp"

#include <algorithm>
#include <glm/gtc/type_ptr.hpp>

Camera::Camera(glm::vec3 position, glm::vec3 up, float yaw, float pitch)
    : Position(position), Front(glm::vec3(0.0f, 0.0f, -1.0f)), WorldUp(up), Yaw(yaw), Pitch(pitch),
      MovementSpeed(SPEED), MouseSensitivity(SENSITIVITY), Zoom(ZOOM), FollowDistance(FOLLOW_DISTANCE), Mode(MODE) {
    updateCameraVectors();
}

Camera::Camera(float posX, float posY, float posZ, float upX, float upY, float upZ, float yaw, float pitch)
    : Front(glm::vec3(0.0f, 0.0f, -1.0f)), MovementSpeed(SPEED), MouseSensitivity(SENSITIVITY), Zoom(ZOOM),
      FollowDistance(FOLLOW_DISTANCE), Mode(MODE) {
    Position = glm::vec3(posX, posY, posZ);
    WorldUp = glm::vec3(upX, upY, upZ);
    Yaw = yaw;
    Pitch = pitch;
    updateCameraVectors();
}

glm::mat4 Camera::GetViewMatrix() const { return glm::lookAt(Position, Position + Front, Up); }

void Camera::ProcessKeyboard(const Camera_Movement direction, const float deltaTime) {
    if (Mode != FREE_ROAM) return;

    const float velocity = MovementSpeed * deltaTime;
    if (direction == FORWARD) Position += Front * velocity;
    if (direction == BACKWARD) Position -= Front * velocity;
    if (direction == LEFT) Position -= Right * velocity;
    if (direction == RIGHT) Position += Right * velocity;
}

void Camera::ProcessMouseMovement(float xOffset, float yOffset, bool constrainPitch) {
    xOffset *= MouseSensitivity;
    yOffset *= MouseSensitivity;

    Yaw += xOffset;
    Pitch += yOffset;

    if (constrainPitch) {
        if (Pitch > 89.0f) Pitch = 89.0f;
        if (Pitch < -89.0f) Pitch = -89.0f;
    }

    updateCameraVectors();
}

void Camera::ProcessMouseScroll(float yOffset) {
    if (Mode == FREE_ROAM) {
        Zoom -= yOffset;
        if (Zoom < 1.0f) Zoom = 1.0f;
        if (Zoom > 45.0f) Zoom = 45.0f;
    } else if (Mode == FOLLOW) {
        FollowDistance -= yOffset;
        FollowDistance = std::clamp(FollowDistance, 5.0f, 25.0f);
    }
}

void Camera::updateCamera(const glm::vec3 &vehiclePosition) {
    if (Mode == FOLLOW) calculateFollowPositions(vehiclePosition);
}

void Camera::setNextCameraMode() {
    Mode = static_cast<Camera_Mode>((static_cast<int>(Mode) + 1) % Camera_Mode::INTERNAL_COUNTER);
}

void Camera::updateCameraVectors() {
    glm::vec3 front;
    front.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
    front.y = sin(glm::radians(Pitch));
    front.z = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));
    Front = glm::normalize(front);
    Right = glm::normalize(glm::cross(Front, WorldUp));
    Up = glm::normalize(glm::cross(Right, Front));
}

void Camera::calculateFollowPositions(const glm::vec3 &vehiclePosition) {
    const float offsetX = -FollowDistance * cos(glm::radians(Pitch)) * cos(glm::radians(Yaw));
    const float offsetY = -FollowDistance * sin(glm::radians(Pitch));
    const float offsetZ = -FollowDistance * cos(glm::radians(Pitch)) * sin(glm::radians(Yaw));

    Position = vehiclePosition + glm::vec3(offsetX, offsetY, offsetZ);

    Front = glm::normalize(vehiclePosition - Position);

    Right = glm::normalize(glm::cross(Front, WorldUp));
    Up = glm::normalize(glm::cross(Right, Front));
}

void Camera::setCameraMode(const Camera_Mode mode) { this->Mode = mode; }

float Camera::getZoom() const { return Zoom; }
