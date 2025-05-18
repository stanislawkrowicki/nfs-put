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

#include "physics_debug.hpp"
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

void processInput(GLFWwindow *window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) camera.ProcessKeyboard(RIGHT, deltaTime);
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

btDiscreteDynamicsWorld *dynamicsWorld;
btRaycastVehicle *vehicle;
btRigidBody *carChassis;

DebugDrawer *debugDrawer;

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

    processInput(window);

    glClearColor(0.05f, 0.05f, 0.05f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // don't forget to enable shader before setting uniforms
    sp->use();

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
    btTransform chassisTrans;
    carChassis->getMotionState()->getWorldTransform(chassisTrans);
    // std::cout << "Car Position: " << pos.getX() << ", " << pos.getY() << ", " << pos.getZ() << std::endl;
    drawCube(chassisTrans, btVector3(1, 0.5, 2));

    if (debugDrawer->isEnabled())
        debugDrawer->draw(projection * view * model);

    for (int i = 0; i < vehicle->getNumWheels(); i++) {
        drawWheel(vehicle->getWheelInfo(i), carShader);
    }
}

void initPhysics(const std::vector<Vertex> &vertices, const std::vector<unsigned int> &indices) {
    btDefaultCollisionConfiguration *collisionConfig = new btDefaultCollisionConfiguration();
    btCollisionDispatcher *dispatcher = new btCollisionDispatcher(collisionConfig);
    btBroadphaseInterface *broadphase = new btDbvtBroadphase();
    btSequentialImpulseConstraintSolver *solver = new btSequentialImpulseConstraintSolver();

    dynamicsWorld = new btDiscreteDynamicsWorld(dispatcher, broadphase, solver, collisionConfig);
    dynamicsWorld->setGravity(btVector3(0, -9.81f, 0));

    auto *triMesh = new btTriangleMesh();

    for (size_t i = 0; i < indices.size(); i += 3) {
        const Vertex &v0 = vertices[indices[i]];
        const Vertex &v1 = vertices[indices[i + 1]];
        const Vertex &v2 = vertices[indices[i + 2]];

        btVector3 bv0(v0.Position.x, v0.Position.y, v0.Position.z);
        btVector3 bv1(v1.Position.x, v1.Position.y, v1.Position.z);
        btVector3 bv2(v2.Position.x, v2.Position.y, v2.Position.z);

        triMesh->addTriangle(bv0, bv1, bv2);
    }

    bool useQuantizedAabbCompression = false;
    btBvhTriangleMeshShape *groundShape = new btBvhTriangleMeshShape(triMesh, useQuantizedAabbCompression);
    groundShape->setMargin(0.02f);

    // Motion state: identity transform (no offset)
    btDefaultMotionState *groundMotion = new btDefaultMotionState(btTransform::getIdentity());

    btRigidBody::btRigidBodyConstructionInfo groundCI(0.0f, groundMotion, groundShape);
    btRigidBody *groundRigidBody = new btRigidBody(groundCI);
    dynamicsWorld->addRigidBody(groundRigidBody);

    // Car chassis
    btCollisionShape *chassisShape = new btBoxShape(btVector3(1, 0.5, 2));
    btTransform chassisTransform;
    chassisTransform.setIdentity();
    chassisTransform.setOrigin(btVector3(0, 2, 3));
    btQuaternion rotation(btVector3(0, 1, 0), SIMD_HALF_PI);
    chassisTransform.setRotation(rotation);

    btScalar mass = 800;
    btVector3 inertia(0, 0, 0);
    chassisShape->calculateLocalInertia(mass, inertia);
    btDefaultMotionState *chassisMotion = new btDefaultMotionState(chassisTransform);
    btRigidBody::btRigidBodyConstructionInfo carCI(mass, chassisMotion, chassisShape, inertia);
    carChassis = new btRigidBody(carCI);
    dynamicsWorld->addRigidBody(carChassis);

    // Vehicle system
    btRaycastVehicle::btVehicleTuning tuning;
    btVehicleRaycaster *raycaster = new btDefaultVehicleRaycaster(dynamicsWorld);
    vehicle = new btRaycastVehicle(tuning, carChassis, raycaster);
    carChassis->setActivationState(DISABLE_DEACTIVATION);
    dynamicsWorld->addVehicle(vehicle);

    debugDrawer = new DebugDrawer(dynamicsWorld);

    btVector3 wheelDir(0, -1, 0);
    btVector3 wheelAxle(1, 0, 0);
    float suspensionRest = 0.6f;
    float wheelRadius = 0.5f;

    vehicle->addWheel(btVector3(-1.5, -0.5, 1.5), wheelDir, wheelAxle, suspensionRest, wheelRadius, tuning, true);
    vehicle->addWheel(btVector3(1.5, -0.5, 1.5), wheelDir, wheelAxle, suspensionRest, wheelRadius, tuning, true);
    vehicle->addWheel(btVector3(-1.5, -0.5, -1.5), wheelDir, wheelAxle, suspensionRest, wheelRadius, tuning, false);
    vehicle->addWheel(btVector3(1.5, -0.5, -1.5), wheelDir, wheelAxle, suspensionRest, wheelRadius, tuning, false);

    for (int i = 0; i < vehicle->getNumWheels(); i++) {
        btWheelInfo &wheel = vehicle->getWheelInfo(i);
        wheel.m_suspensionStiffness = 20;
        wheel.m_wheelsDampingRelaxation = 2.3f;
        wheel.m_wheelsDampingCompression = 4.4f;
        wheel.m_frictionSlip = 1000;
        wheel.m_rollInfluence = 0.1f;
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


    // auto positions = getBtPositionsFromVertices(vertices);

    setupCubeGeometry();
    setupWheelGeometry();
    initPhysics(vertices, indices);

    while (!glfwWindowShouldClose(window)) {
        dynamicsWorld->stepSimulation(1.0f / 60.0f);

        // Always forward
        vehicle->applyEngineForce(1000.f, 2);
        vehicle->applyEngineForce(1000.f, 3);
        vehicle->setSteeringValue(0.0f, 0);
        vehicle->setSteeringValue(0.0f, 1);
        drawScene(window);
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwDestroyWindow(window);
    glfwTerminate();

    delete trackModel;

    exit(EXIT_SUCCESS);
}
