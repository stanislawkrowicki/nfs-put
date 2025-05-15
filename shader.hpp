#pragma once

#include <string>
#include "GL/glew.h"
#include "glm/glm.hpp"

class Shader {
    GLuint vertexShader;
    GLuint geometryShader;
    GLuint fragmentShader;

    static GLuint loadShaderWithType(const std::string& shaderPath, GLenum type);

  public:
    /** Loaded shader ID */
    unsigned int shaderID;

    /** Loads shaders from provided .glsl files. If you want to omit geometryShader, set it to NULL */
    Shader(const std::string& vertexShaderPath, const std::string* geometryShaderPath, const std::string& fragmentShaderPath);

    ~Shader();

    /** Activate the shader */
    void use() const;

    /** Returns location of attribute with name attribName inside a .glsl */
    [[nodiscard]]
    GLint a(const std::string &attribName) const;

    /** Returns location of uniform with name uniformName inside a .glsl */
    [[nodiscard]]
    GLint u(const std::string &uniformName) const;

    void setUniform(const std::string& name, int value) const;
    void setUniform(const std::string& name, float value) const;
    void setUniform(const std::string& name, const glm::vec3& value) const;
    void setUniform(const std::string& name, const glm::vec4& value) const;
    void setUniform(const std::string& name, const glm::mat4& value, bool transpose = false) const;
};