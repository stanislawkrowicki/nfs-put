#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <iostream>
#include <assimp/scene.h>
#include "camera.hpp"
#include <shader.hpp>
#include "model.hpp"
#include "btBulletDynamicsCommon.h"
#include "BulletDynamics/Vehicle/btRaycastVehicle.h"
#include <vector>
#include <cmath>

#include "physics.hpp"
#include "physics_debug.hpp"
#include "vehicle.hpp"
#include "vehicle_manager.hpp"
#include "glm/gtc/type_ptr.hpp"

Shader *sp;
Shader *carShader;

void errorCallback(int error, const char *description) { fputs(description, stderr); }

Camera camera(glm::vec3(0.0f, 5.0f, 0.0f));
Model *trackModel;
float  lastX = 800.0f / 2.0f;
float  lastY = 600.0f / 2.0f;
bool   firstMouse = true;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

std::shared_ptr<Vehicle> playerVehicle;

void processInput(GLFWwindow *window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_I) == GLFW_PRESS) camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_K) == GLFW_PRESS) camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_J) == GLFW_PRESS) camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS) camera.ProcessKeyboard(RIGHT, deltaTime);
}

void processKeyCallbacks(GLFWwindow *window, const int key, const int scancode, const int action, const int mods) {
    if (key == GLFW_KEY_V && action == GLFW_PRESS)
        camera.setNextCameraMode();
}

void processVehicleInputs(GLFWwindow *window, const std::shared_ptr<Vehicle> &vehicle, const float deltaTime) {
    const bool left = glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS;
    const bool right = glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS;
    const bool handbrake = glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS;
    const bool forward = glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS;
    const bool backward = glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS;

    vehicle->updateControls(forward, backward, handbrake, left, right, deltaTime);
}

void framebuffer_size_callback(GLFWwindow *window, int width, int height) { glViewport(0, 0, width, height); }

void mouse_callback(GLFWwindow *window, double xposIn, double yposIn) {
    auto xPos = static_cast<float>(xposIn);
    auto yPos = static_cast<float>(yposIn);

    if (firstMouse) {
        lastX = xPos;
        lastY = yPos;
        firstMouse = false;
    }

    float xOffset = xPos - lastX;
    float yOffset = lastY - yPos; // reversed since y-coordinates go from bottom to top

    lastX = xPos;
    lastY = yPos;

    camera.ProcessMouseMovement(xOffset, yOffset);
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow *window, double xOffset, double yOffset) {
    camera.ProcessMouseScroll(static_cast<float>(yOffset));
}

GLuint cubeVAO, cubeVBO;

void setupCubeGeometry() {
    float vertices[] = {
        // front face
        -0.5f, -0.5f, 0.5f, 0.5f, -0.5f, 0.5f,
        0.5f, 0.5f, 0.5f, -0.5f, 0.5f, 0.5f,
        // back face
        -0.5f, -0.5f, -0.5f, -0.5f, 0.5f, -0.5f,
        0.5f, 0.5f, -0.5f, 0.5f, -0.5f, -0.5f,
        // left face
        -0.5f, -0.5f, -0.5f, -0.5f, -0.5f, 0.5f,
        -0.5f, 0.5f, 0.5f, -0.5f, 0.5f, -0.5f,
        // right face
        0.5f, -0.5f, -0.5f, 0.5f, 0.5f, -0.5f,
        0.5f, 0.5f, 0.5f, 0.5f, -0.5f, 0.5f,
        // top face
        -0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f,
        0.5f, 0.5f, -0.5f, -0.5f, 0.5f, -0.5f,
        // bottom face
        -0.5f, -0.5f, 0.5f, -0.5f, -0.5f, -0.5f,
        0.5f, -0.5f, -0.5f, 0.5f, -0.5f, 0.5f,
    };

    glGenVertexArrays(1, &cubeVAO);
    glGenBuffers(1, &cubeVBO);
    glBindVertexArray(cubeVAO);
    glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), nullptr);
    glBindVertexArray(0);
}

void drawCube(const btTransform &trans, const btVector3 &halfExtents) {
    btScalar mat[16];
    trans.getOpenGLMatrix(mat);
    glm::mat4 model = glm::make_mat4(mat);
    model = glm::scale(model, glm::vec3(halfExtents.x() * 2, halfExtents.y() * 2, halfExtents.z() * 2));

    carShader->use();
    carShader->setUniform("M", model);
    carShader->setUniform("color", glm::vec3(0.0f, 0.0f, 1.0f));
    glBindVertexArray(cubeVAO);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 24);
    glBindVertexArray(0);
}

GLuint wheelVAO = 0, wheelVBO = 0;
int wheelVertexCount = 0;

void setupWheelGeometry(int segments = 24) {
    std::vector<float> vertices;
    float radius = 0.5f;
    float halfLength = 0.1f; // wheel width

    for (int i = 0; i <= segments; ++i) {
        float theta = 2.0f * M_PI * i / segments;
        float x = cos(theta) * radius;
        float y = sin(theta) * radius;

        vertices.push_back(x);
        vertices.push_back(y);
        vertices.push_back(-halfLength); // bottom
        vertices.push_back(x);
        vertices.push_back(y);
        vertices.push_back(halfLength); // top
    }

    wheelVertexCount = static_cast<int>(vertices.size()) / 3;

    glGenVertexArrays(1, &wheelVAO);
    glGenBuffers(1, &wheelVBO);
    glBindVertexArray(wheelVAO);
    glBindBuffer(GL_ARRAY_BUFFER, wheelVBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), nullptr);
    glBindVertexArray(0);
}

void drawWheel(const btWheelInfo &wheel, Shader *shader) {
    btTransform trans = wheel.m_worldTransform;
    btScalar mat[16];
    trans.getOpenGLMatrix(mat);

    glm::mat4 model = glm::make_mat4(mat);
    model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0, 1, 0)); // rotate to lie flat
    model = glm::scale(model, glm::vec3(wheel.m_wheelsRadius, wheel.m_wheelsRadius, 1.0f));

    shader->use();
    shader->setUniform("M", model);
    shader->setUniform("color", glm::vec3(1.0f, 0.0f, 0.0f)); // red

    glBindVertexArray(wheelVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, wheelVertexCount);
    glBindVertexArray(0);
}

void drawScene(GLFWwindow *window) {
    const auto currentFrame = static_cast<float>(glfwGetTime());
    deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;

    glClearColor(0.05f, 0.05f, 0.05f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // don't forget to enable shader before setting uniforms
    sp->use();

    const auto chassisOrigin = playerVehicle->getBtVehicle()->getChassisWorldTransform().getOrigin();
    const auto vehPos = new glm::vec3();
    vehPos->x = chassisOrigin.getX();
    vehPos->y = chassisOrigin.getY();
    vehPos->z = chassisOrigin.getZ();

    camera.updateCamera(*vehPos);

    // view/projection transformations
    glm::mat4 projection = glm::perspective(glm::radians(camera.getZoom()), 800.0f / 600.0f, 0.1f, 1000.0f);
    glm::mat4 view = camera.GetViewMatrix();

    sp->setUniform("P", projection);
    sp->setUniform("V", view);
    // render the loaded model
    auto model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f)); // translate it down so it's at the center of the scene
    sp->setUniform("M", model);
    trackModel->Draw(*sp);

    carShader->use();
    carShader->setUniform("V", view);
    carShader->setUniform("P", projection);

    // Draw chassis
    for (const auto &vehicle: VehicleManager::getInstance().getVehicles()) {
        const auto config = vehicle->getConfig();
        const auto chassisTrans = vehicle->getBtVehicle()->getChassisWorldTransform();
        drawCube(chassisTrans, config.chassisHalfExtents);

        for (int i = 0; i < vehicle->getBtVehicle()->getNumWheels(); ++i) {
            drawWheel(vehicle->getBtVehicle()->getWheelInfo(i), carShader);
        }
    }
}

int main() {
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        exit(EXIT_FAILURE);
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    glfwSetErrorCallback(errorCallback);
    GLFWwindow *window = glfwCreateWindow(800, 600, "NFS PUT", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetKeyCallback(window, processKeyCallbacks);

    // tell GLFW to capture our mouse
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    /* VSync */
    glfwSwapInterval(1);

    GLenum glewErr;
    if ((glewErr = glewInit()) != GLEW_OK) {
        std::cerr << "Can't initialize GLEW: " << glewGetErrorString(glewErr);
    }

    glEnable(GL_DEPTH_TEST);

    trackModel = new Model("spielberg.glb", true);
    sp = new Shader("textured_vert.glsl", nullptr, "textured_frag.glsl");
    carShader = new Shader("simplest_vert.glsl", nullptr, "simplest_frag.glsl");
    // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    // glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    auto meshes = trackModel->getMeshes();

    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;

    unsigned int vertexOffset = 0;

    for (const auto &mesh: meshes) {
        vertices.insert(vertices.end(), mesh.vertices.begin(), mesh.vertices.end());

        for (unsigned int index: mesh.indices) {
            indices.push_back(index + vertexOffset);
        }

        vertexOffset += mesh.vertices.size();
    }

    setupCubeGeometry();
    setupWheelGeometry();

    auto &physics = Physics::getInstance();
    const auto triMesh = Physics::btTriMeshFromModel(vertices, indices);
    physics.initPhysics(triMesh);
    const VehicleConfig defaultConfig;
    defaultConfig.rotation = btQuaternion(btVector3(0, -1, 0), SIMD_HALF_PI);

    playerVehicle = VehicleManager::getInstance().createVehicle(defaultConfig);

    while (!glfwWindowShouldClose(window)) {
        physics.stepSimulation(deltaTime);

        processInput(window);
        processVehicleInputs(window, playerVehicle, deltaTime);

        drawScene(window);
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwDestroyWindow(window);
    glfwTerminate();

    delete trackModel;

    exit(EXIT_SUCCESS);
}
