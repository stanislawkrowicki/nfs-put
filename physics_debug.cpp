#include "physics_debug.hpp"
#include <iostream>

DebugDrawer::DebugDrawer(btDynamicsWorld *dynamicsWorld) : enabled(false),
                                                           m_debugMode(DBG_DrawWireframe),
                                                           dynamicsWorld(dynamicsWorld),
                                                           vertexColor(glm::vec3(0.0f, 1.0f, 0.0f)) {
    shader = new Shader("physicsdebug_vert.glsl", nullptr, "physicsdebug_frag.glsl");

    dynamicsWorld->setDebugDrawer(this);
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);

    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

bool DebugDrawer::isEnabled() const {
    return enabled;
}

void DebugDrawer::enable() {
    enabled = true;
}

void DebugDrawer::disable() {
    enabled = false;
}

void DebugDrawer::clear() {
    lineVertices.clear();
}

void DebugDrawer::drawLine(const btVector3 &from, const btVector3 &to, const btVector3 &color) {
    lineVertices.push_back(from.x());
    lineVertices.push_back(from.y());
    lineVertices.push_back(from.z());

    lineVertices.push_back(to.x());
    lineVertices.push_back(to.y());
    lineVertices.push_back(to.z());
}

void DebugDrawer::reportErrorWarning(const char *warningString) {
    std::cerr << "BULLET:ERROR:WARNING:   " << warningString << std::endl;
}

void DebugDrawer::setDebugMode(int debugMode) {
    m_debugMode = debugMode;
}

int DebugDrawer::getDebugMode() const {
    return m_debugMode;
}

void DebugDrawer::setVertexColor(const glm::vec3 &color) {
    vertexColor = color;
}

void DebugDrawer::draw(const glm::mat4 &PVM) {
    if (!enabled) {
        return;
    }

    dynamicsWorld->debugDrawWorld();

    shader->use();
    shader->setUniform("PVM", PVM);
    shader->setUniform("color", vertexColor);

    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);

    glBufferData(GL_ARRAY_BUFFER, static_cast<long>(lineVertices.size() * sizeof(float)), lineVertices.data(),
                 GL_DYNAMIC_DRAW);

    glDrawArrays(GL_LINES, 0, static_cast<GLsizei>(lineVertices.size() / 3));

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    /* TODO: add Shader::disable() */
    glUseProgram(0);

    clear();
}
