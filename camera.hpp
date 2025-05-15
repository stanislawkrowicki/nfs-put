#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

/* Defines several possible options for camera movement. */
enum Camera_Movement {
    FORWARD,
    BACKWARD,
    LEFT,
    RIGHT
};

/* Defines several camera modes */
enum Camera_Mode {
    FREE_ROAM
};

/* Default camera settings */
constexpr float YAW         = -90.0f;
constexpr float PITCH       =  0.0f;
constexpr float SPEED       =  25.0f;
constexpr float SENSITIVITY =  0.1f;
constexpr float ZOOM        =  45.0f;
constexpr Camera_Mode MODE  = FREE_ROAM;

class Camera
{
    /* Camera Attributes */
    glm::vec3 Position{};
    glm::vec3 Front;
    glm::vec3 Up{};
    glm::vec3 Right{};
    glm::vec3 WorldUp{};

    /* Euler Angles */
    float Yaw;
    float Pitch;

    /* Options */
    float MovementSpeed;
    float MouseSensitivity;
    float Zoom;
    Camera_Mode Mode;

public:
    /* Constructor by vectors */
    explicit Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f),
           glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f),
           float yaw = YAW, float pitch = PITCH);

    /* Constructor by scalars */
    Camera(float posX, float posY, float posZ,
           float upX, float upY, float upZ,
           float yaw, float pitch);

    [[nodiscard]]
    glm::mat4 GetViewMatrix() const;

    void ProcessKeyboard(Camera_Movement direction, float deltaTime);
    void ProcessMouseMovement(float xOffset, float yOffset, bool constrainPitch = true);
    void ProcessMouseScroll(float yOffset);
    void setCameraMode(Camera_Mode mode);

    [[nodiscard]]
    float getZoom() const;
private:
    void updateCameraVectors();
};
