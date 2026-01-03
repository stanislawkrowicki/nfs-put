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

#include "opponent.hpp"
#include "physics.hpp"
#include "physics_debug.hpp"
#include "vehicle.hpp"
#include "vehicle_manager.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "skybox.hpp"
#include "opponent_path.hpp"
#include "debug.hpp"
#include "netcode/client/udp_client.hpp"
#include <chrono>
#include <thread>

#include "default_vehicle_model.hpp"
#include "laps.hpp"
#include "netcode/shared/starting_positions.hpp"
#include "netcode/client/opponent_manager.hpp"
#include "netcode/shared/client_inputs.hpp"
#include "netcode/client/tcp_client.hpp"
#include "netcode/shared/packets/tcp/client/client_game_loaded_packet.hpp"
#include "netcode/shared/packets/tcp/client/udp_info_packet.hpp"
#include "netcode/shared/packets/udp/client/ping_packet.hpp"

using namespace std::chrono;

Shader *simpleShader;
Shader *carShader;
Shader *trackShader;

void errorCallback(int error, const char *description) { fputs(description, stderr); }

Camera camera(glm::vec3(0.0f, 5.0f, 0.0f));
Model *trackModel;
Model *wheelModel;
float  lastX = 800.0f / 2.0f;
float  lastY = 600.0f / 2.0f;
bool   firstMouse = true;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

std::shared_ptr<Vehicle> playerVehicle;
std::shared_ptr<Vehicle> opponentVehicle;
OpponentPathGenerator *pathGenerator;
Opponent *opponent;

/* Switching between windowed and fullscreen */
constexpr float DEFAULT_WINDOW_WIDTH = 800.0f, DEFAULT_WINDOW_HEIGHT = 600.0f;
bool isFullscreen = false;
int windowedX, windowedY, windowedWidth, windowedHeight;
float currentWindowWidth = DEFAULT_WINDOW_WIDTH, currentWindowHeight = DEFAULT_WINDOW_HEIGHT;

void toggleFullscreen(GLFWwindow *window) {
    isFullscreen = !isFullscreen;

    GLFWmonitor *primaryMonitor = glfwGetPrimaryMonitor();
    const GLFWvidmode *mode = glfwGetVideoMode(primaryMonitor);

    if (isFullscreen) {
        glfwGetWindowPos(window, &windowedX, &windowedY);
        glfwGetWindowSize(window, &windowedWidth, &windowedHeight);

        glfwSetWindowMonitor(window, primaryMonitor,
                             0, 0, mode->width, mode->height,
                             mode->refreshRate);
        currentWindowWidth = static_cast<float>(mode->width);
        currentWindowHeight = static_cast<float>(mode->height);
    } else {
        glfwSetWindowMonitor(window, nullptr,
                             windowedX, windowedY, windowedWidth, windowedHeight,
                             mode->refreshRate);
        currentWindowWidth = static_cast<float>(windowedWidth);
        currentWindowHeight = static_cast<float>(windowedHeight);
    }
}

void processInput(GLFWwindow *window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_I) == GLFW_PRESS) camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_K) == GLFW_PRESS) camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_J) == GLFW_PRESS) camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS) camera.ProcessKeyboard(RIGHT, deltaTime);
}

void processKeyCallbacks(GLFWwindow *window, const int key, const int scancode, const int action, const int mods) {
    if (key == GLFW_KEY_V && action == GLFW_PRESS) camera.setNextCameraMode();
    if (key == GLFW_KEY_F6 && action == GLFW_PRESS) Physics::getInstance().getDebugDrawer()->toggle();
    if (key == GLFW_KEY_F11 && action == GLFW_PRESS) toggleFullscreen(window);
    if (key == GLFW_KEY_X && action == GLFW_PRESS) pathGenerator->addWaypointFromVehicle(playerVehicle);
    if (key == GLFW_KEY_F10 && action == GLFW_PRESS)
        pathGenerator->saveWaypointsToFile("paths.json");
    if (key == GLFW_KEY_F7 && action == GLFW_PRESS) playerVehicle->printDebugPosition();
}

void processVehicleInputs(GLFWwindow *window, const std::shared_ptr<Vehicle> &vehicle, const float deltaTime) {
    const bool left = glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS;
    const bool right = glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS;
    const bool handbrake = glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS;
    const bool forward = glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS;
    const bool backward = glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS;

    vehicle->updateControls(forward, backward, handbrake, left, right, deltaTime);
}

void framebuffer_size_callback(GLFWwindow *window, int width, int height) {
    glViewport(0, 0, width, height);
    currentWindowHeight = static_cast<float>(height);
    currentWindowWidth = static_cast<float>(width);
}

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

    simpleShader->use();
    simpleShader->setUniform("M", model);
    simpleShader->setUniform("color", glm::vec3(0.0f, 0.0f, 1.0f));
    glBindVertexArray(cubeVAO);

    for (int i = 0; i < 6; i++) {
        glDrawArrays(GL_TRIANGLE_FAN, i * 4, 4);
    }

    glBindVertexArray(0);
}

void drawWaypoint(const glm::vec3 &position, const Shader *shader) {
    constexpr auto halfExtents = glm::vec3(0.5, 0.5, 0.5);

    auto model = glm::mat4(1.0);
    model = glm::translate(model, position);
    model = glm::scale(model, halfExtents * 2.0f);

    /* Needs to be already in use with specified V and P matrices! */
    // shader->use();

    shader->setUniform("M", model);
    shader->setUniform("color", glm::vec3(0.0f, 0.0f, 1.0f));
    glBindVertexArray(cubeVAO);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 24);
    glBindVertexArray(0);
}

GLuint wheelVAO = 0, wheelVBO = 0;
int wheelVertexCount = 0;

void setupWheelGeometry(int segments = 24) {
    std::vector<float> vertices;
    float radius = 0.9f;
    float halfLength = 0.15f; // wheel width

    for (int i = 0; i <= segments; ++i) {
        float theta = 2.0f * 3.14 * i / segments;
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

void drawWheel(const btWheelInfo &wheel, Shader *shader, const int wheelID, const float rollingRotation) {
    const btTransform trans = wheel.m_worldTransform;
    btScalar mat[16];
    trans.getOpenGLMatrix(mat);
    glm::mat4 model = glm::make_mat4(mat);
    model = glm::scale(model, glm::vec3(1.4f, 1.4f, 1.4f));

    if (wheelID == 0 || wheelID == 2) {
        /* Right wheels. Hacky. */
        model = glm::rotate(model, glm::radians(180.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        model = glm::translate(model, glm::vec3(0.035f, 0.0f, 0.0f));
    } else {
        model = glm::translate(model, glm::vec3(0.068f, 0.0f, 0.0f));
    }

    model = glm::rotate(model, rollingRotation, glm::vec3(1.0f, 0.0f, 0.0f));

    shader->setUniform("M", model);

    wheelModel->Draw(*shader);
}

void drawScene(GLFWwindow *window) {
    const auto currentFrame = static_cast<float>(glfwGetTime());
    deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;
    if (deltaTime > 0.1f) deltaTime = 0.1f;
    processInput(window);

    glClearColor(0.1f, 0.8f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    const auto chassisOrigin = playerVehicle->getBtVehicle()->getChassisWorldTransform().getOrigin();
    const auto vehPos = new glm::vec3();
    vehPos->x = chassisOrigin.getX();
    vehPos->y = chassisOrigin.getY();
    vehPos->z = chassisOrigin.getZ();

    btTransform transform = playerVehicle->getBtVehicle()->getChassisWorldTransform();
    btMatrix3x3 rotMatrix = transform.getBasis();

    //Car has different Y and Z axis
    float vehYaw = atan2(rotMatrix[0][0], rotMatrix[0][2]);
    vehYaw = glm::degrees(vehYaw);

    btVector3 linearVelocity = playerVehicle->getBtVehicle()->getRigidBody()->getLinearVelocity();
    float vehicleSpeed = linearVelocity.length();

    camera.updateCamera(*vehPos);

    // view/projection transformations
    const auto aspectRatio = currentWindowWidth / currentWindowHeight;
    const glm::mat4 projection = glm::perspective(glm::radians(camera.getZoom()), aspectRatio,
                                                  0.1f, 1000.0f);
    const glm::mat4 view = camera.GetViewMatrix();

    Skybox::draw(view, projection);

    std::vector<glm::vec3> brakeLightPositions;
    std::vector<glm::vec3> brakeLightDirections;
    int brakeLightCount = 0;
    /* Limit for the shader */
    constexpr int brakeLightLimit = 8;

    // Draw chassis
    for (const auto &vehicle: VehicleManager::getInstance().getVehicles()) {
        const auto config = vehicle->getConfig();
        // const auto chassisTrans = vehicle->getBtVehicle()->getChassisWorldTransform();
        const auto vehicleModel = vehicle->getModel();

        glm::mat4 modelMatrix = vehicle->getOpenGLModelMatrix();
        const auto vehiclePos = modelMatrix[3];
        const auto forwardVector = modelMatrix[2];

        if (vehicle->getIsBraking() && brakeLightCount <= brakeLightLimit - 2) {
            brakeLightCount += 2;
            brakeLightPositions.emplace_back(modelMatrix * glm::vec4(config.brakeLights[0], 1.0f));
            brakeLightPositions.emplace_back(modelMatrix * glm::vec4(config.brakeLights[1], 1.0f));
            brakeLightDirections.emplace_back(-forwardVector);
            brakeLightDirections.emplace_back(-forwardVector);
        }

        carShader->use();
        carShader->setUniform("V", view);
        carShader->setUniform("P", projection);
        carShader->setUniform("M", modelMatrix);
        carShader->setUniform("u_braking", vehicle->getIsBraking());

        carShader->setUniform("u_lightColor", glm::vec3(1.0f, 0.95f, 0.95f));
        carShader->setUniform("u_lightPos", glm::vec3(-200.0f, 300.0f, 20.0f));
        carShader->setUniform("u_lightIntensity", 0.85f);
        carShader->setUniform("u_camPos", glm::inverse(view)[3]);

        carShader->setUniform("u_bodyColor", config.bodyColor);
        vehicleModel->Draw(*carShader);

        glDisable(GL_CULL_FACE);
        for (int i = 0; i < vehicle->getBtVehicle()->getNumWheels(); ++i) {
            const float speed = vehicle->getBtVehicle()->getCurrentSpeedKmHour() / 3.6f;
            const float distanceTraveled = speed * deltaTime;
            const float deltaRotation = distanceTraveled / config.wheelRadius;
            const float wheelRotation = vehicle->applyRotationToWheel(i, deltaRotation);

            drawWheel(vehicle->getBtVehicle()->getWheelInfo(i), carShader, i, wheelRotation);
        }
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
    }

    trackShader->use();

    constexpr auto model = glm::mat4(1.0f);
    trackShader->setUniform("P", projection);
    trackShader->setUniform("V", view);
    trackShader->setUniform("M", model);

    trackShader->setUniform("u_lightColor", glm::vec3(1.0f, 0.95f, 0.95f));
    trackShader->setUniform("u_lightPos", glm::vec3(-200.0f, 300.0f, 20.0f));
    trackShader->setUniform("u_lightIntensity", 0.85f);
    trackShader->setUniform("u_camPos", glm::inverse(view)[3]);

    trackShader->setUniform("u_brakeLightCount", brakeLightCount);

    if (brakeLightCount > 0) {
        glUniform3fv(trackShader->u("u_brakeLightPositions[0]"), brakeLightLimit,
                     glm::value_ptr(brakeLightPositions[0]));
        glUniform3fv(trackShader->u("u_brakeLightDirections[0]"), brakeLightLimit,
                     glm::value_ptr(brakeLightDirections[0]));
    }

    trackModel->Draw(*trackShader);

    const auto debugDrawer = Physics::getInstance().getDebugDrawer();

    if (debugDrawer->isEnabled())
        debugDrawer->draw(projection * view * model);

    // simpleShader->use();
    // simpleShader->setUniform("V", view);
    // simpleShader->setUniform("P", projection);
    // for (const auto &waypoint: opponent->waypoints) {
    //     drawWaypoint(waypoint, simpleShader);
    // }
}

int main() {

    auto state = std::make_shared<ClientState>();
    auto tcpClient = std::make_shared<TCPClient>(state);
    std::thread tcpListenThread([tcpClient] {
        tcpClient->connect("127.0.0.1","1313");
    });
    {
        std::unique_lock<std::mutex> lock(state->mtx);
        state->cv.wait(lock, [&] { return state->ready; });
    }
    const auto udpClient = std::make_shared<UDPClient>();

    auto udpPort = udpClient->getPort();
    auto udpInfoPacket = UdpInfoPacket();
    udpInfoPacket.port = udpPort;

    tcpClient->send(TCPPacket::serialize(udpInfoPacket), sizeof(udpInfoPacket));

    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        exit(EXIT_FAILURE);
    }


    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_CONTEXT_DEBUG, true);

    int flags;
    glGetIntegerv(GL_CONTEXT_FLAGS, &flags);
    if (flags & GL_CONTEXT_FLAG_DEBUG_BIT) {
        glEnable(GL_DEBUG_OUTPUT);
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
        glDebugMessageCallback(glDebugOutput, nullptr);
        glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
    }

    glfwWindowHint(GLFW_DEPTH_BITS, 32);

    glfwSetErrorCallback(errorCallback);
    GLFWwindow *window = glfwCreateWindow(DEFAULT_WINDOW_WIDTH, DEFAULT_WINDOW_HEIGHT, "NFS PUT", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    glfwMakeContextCurrent(window);

    int fbWidth, fbHeight;
    glfwGetFramebufferSize(window, &fbWidth, &fbHeight);
    glViewport(0, 0, fbWidth, fbHeight);
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
    glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    /* If we add windows to vehicle model or anything needing two-sided rendering
     * this needs to be disabled before drawing them */
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    trackModel = new Model("spielberg.glb", true);
    wheelModel = new Model("wheel.glb", false);
    simpleShader = new Shader("simplest_vert.glsl", nullptr, "simplest_frag.glsl");
    trackShader = new Shader("track_vert.glsl", nullptr, "track_frag.glsl");
    carShader = new Shader("car_vert.glsl", nullptr, "car_frag.glsl");

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

    // const auto vehicleModel = std::make_shared<Model>("skyline.glb", true,
    //                                                   aiProcess_Triangulate |
    //                                                   aiProcess_PreTransformVertices |
    //                                                   aiProcess_JoinIdenticalVertices |
    //                                                   aiProcess_ImproveCacheLocality |
    //                                                   aiProcess_GenSmoothNormals |
    //                                                   aiProcess_SortByPType
    // );

    setupCubeGeometry();
    setupWheelGeometry();

    auto &physics = Physics::getInstance();
    const auto triMesh = Physics::btTriMeshFromModel(vertices, indices);
    physics.initPhysics(triMesh);

    const VehicleConfig defaultConfig;
    const auto gridPositionIndex = tcpClient->getGridPosition();
    const auto gridPosition = startingPositions[gridPositionIndex % std::size(startingPositions)];

    defaultConfig.position = gridPosition.getOrigin();
    defaultConfig.rotation = gridPosition.getRotation();

    const auto vehicleModel = VehicleModelCache::getDefaultVehicleModel();

    playerVehicle = VehicleManager::getInstance().createVehicle(defaultConfig, vehicleModel);
    playerVehicle->freeze();

    std::thread udpListenThread([udpClient] {
        const auto packet = UDPPacket::create<PingPacket>(0, nullptr, 0);
        udpClient->send(UDPPacket::serialize(packet), sizeof(PingPacket));
        udpClient->listen();
    });

    // const VehicleConfig opponentConfig;
    // opponentConfig.rotation = btQuaternion(btVector3(0, -1, 0), SIMD_HALF_PI);
    // opponentConfig.isPlayerVehicle = false;
    // opponentConfig.engineForce /= 3.0f;
    // opponentConfig.brakingForce *= 0.4;
    // // opponentConfig.rollInfluence /= 10.0;
    // opponentConfig.frictionSlip = 20.0f;
    // opponentConfig.maxSteeringAngle *= 0.8;
    // opponentConfig.boostStrength *= 1.4;
    // opponentConfig.bodyColor = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
    // // opponentConfig.centerOfMassOffset *= 2.0f;
    //
    //
    // opponentVehicle = VehicleManager::getInstance().createVehicle(opponentConfig, vehicleModel);
    Skybox::init();

    OpponentPathGenerator::getInstance().loadPathsToMemory("paths.json");

    // opponent = new Opponent(opponentVehicle);

    auto &opponentManager = OpponentManager::getInstance();
    opponentManager.setOpenGLReady();
    tcpClient->send(TCPPacket::serialize(ClientGameLoadedPacket()), sizeof(ClientGameLoadedPacket));

    auto &lapsInstance = Laps::getInstance();
    lapsInstance.initializeTracker(physics.getDynamicsWorld());
    lapsInstance.addLocalPlayer(playerVehicle->getBtChassis());

    bool didStart = false;

    auto lastTick = steady_clock::now();
    while (!glfwWindowShouldClose(window)) {
        physics.stepSimulation(deltaTime);

        if (!didStart && tcpClient->isRaceStartCountdownActive() && tcpClient->getTimeUntilRaceStart() == 0) {
            playerVehicle->unfreeze();
            didStart = true;
        }

        // playerVehicle->getBtVehicle()->updateVehicle(deltaTime);
        if (steady_clock::now() - lastTick > milliseconds(1000 / 32)) {
            const bool left = glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS;
            const bool right = glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS;
            const bool handbrake = glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS;
            const bool forward = glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS;
            const bool backward = glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS;

            const auto inputBitmap = buildInputBitmap(left, right, handbrake, forward, backward);

            udpClient->sendVehicleState(playerVehicle, inputBitmap);
            lastTick = steady_clock::now();
        }

        processInput(window);
        processVehicleInputs(window, playerVehicle, deltaTime);

        opponentManager.applyLastInputs(deltaTime);
        // opponent->updateSteering();

        drawScene(window);

        lapsInstance.updateLocalPlayer();

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwDestroyWindow(window);
    glfwTerminate();

    delete trackModel;

    exit(EXIT_SUCCESS);
}





