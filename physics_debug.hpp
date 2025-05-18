#pragma once
#include <shader.hpp>

#include "LinearMath/btIDebugDraw.h"
#include <glm/glm.hpp>

#include "BulletDynamics/Dynamics/btDiscreteDynamicsWorld.h"

class DebugDrawer : public btIDebugDraw {
    bool enabled;

    int m_debugMode;

    GLuint vao = 0, vbo = 0;
    btDynamicsWorld *dynamicsWorld;
    std::vector<float> lineVertices{};
    glm::vec3 vertexColor;

    Shader *shader{};

public:
    explicit DebugDrawer(btDynamicsWorld *dynamicsWorld);

    [[nodiscard]]
    bool isEnabled() const;

    void enable();

    void disable();

    void clear();

    void drawContactPoint(const btVector3 &PointOnB, const btVector3 &normalOnB, btScalar distance, int lifeTime,
                          const btVector3 &color) override {
        /* Not implemented */
    };

    void draw3dText(const btVector3 &location, const char *textString) override {
        /* Not implemented */
    };

    void drawLine(const btVector3 &from, const btVector3 &to, const btVector3 &color) override;

    void reportErrorWarning(const char *warningString) override;

    /** For debugMode see btIDebugDraw::DebugDrawModes */
    void setDebugMode(int debugMode) override;

    [[nodiscard]]
    int getDebugMode() const override;

    void setVertexColor(const glm::vec3 &color);

    void draw(const glm::mat4 &PVM);
};
