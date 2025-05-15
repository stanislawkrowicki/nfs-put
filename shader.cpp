#include "shader.hpp"
#include <fstream>
#include <sstream>
#include <iostream>
#include "glm/gtc/type_ptr.hpp"

GLuint Shader::loadShaderWithType(const std::string& shaderPath, GLenum type) {
    std::string shaderContent;

    try {
        std::ifstream shaderFile;
        shaderFile.exceptions (std::ifstream::failbit | std::ifstream::badbit);

        shaderFile.open(std::string(SHADERS_PATH) + shaderPath);
        std::stringstream shaderStream;
        shaderStream << shaderFile.rdbuf();

        shaderFile.close();
        shaderContent = shaderStream.str();
    } catch (std::ifstream::failure& e) {
        std::cerr << "ERROR::SHADER::FILE_READ_ERROR: " << e.what() << std::endl;
        exit(1);
    }

    const char* shaderCode = shaderContent.c_str();
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &shaderCode, nullptr);
    glCompileShader(shader);

    int success;
    int infoLogLength = 0;
    int charsWritten = 0;

    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLogLength);

    if (infoLogLength > 1) {
        char *infoLog;
        infoLog = new char[infoLogLength];
        glGetShaderInfoLog(shader, infoLogLength, &charsWritten, infoLog);
        std::cout << infoLog << std::endl;
        delete []infoLog;
    }

    if (!success) {
        std::cerr << "ERROR! Compilation error for: " << shaderPath << std::endl;
        exit(1);
    }

    return shader;
}

Shader::Shader(const std::string& vertexShaderPath, const std::string* geometryShaderPath, const std::string& fragmentShaderPath) {
    const bool hasGShader = geometryShaderPath != nullptr;

    std::cout << "Loading shader " << vertexShaderPath << std::endl;
    vertexShader = loadShaderWithType(vertexShaderPath, GL_VERTEX_SHADER);

    if (hasGShader) {
        std:: cout << "Loading shader " << *geometryShaderPath << std::endl;
        geometryShader = loadShaderWithType(*geometryShaderPath, GL_GEOMETRY_SHADER);
    } else {
        geometryShader = 0;
    }

    std::cout << "Loading shader " << fragmentShaderPath << std::endl;
    fragmentShader = loadShaderWithType(fragmentShaderPath, GL_FRAGMENT_SHADER);

    shaderID = glCreateProgram();
    glAttachShader(shaderID, vertexShader);
    if (hasGShader) glAttachShader(shaderID, geometryShader);
    glAttachShader(shaderID, fragmentShader);

    glLinkProgram(shaderID);

    int success;
    glGetProgramiv(shaderID, GL_LINK_STATUS, &success);

    if (!success) {
        int infoLogLength;

        glGetProgramiv(shaderID, GL_INFO_LOG_LENGTH, &infoLogLength);
        const auto infoLog = new char[infoLogLength];
        glGetProgramInfoLog(shaderID, 512, nullptr, infoLog);

        std::cerr << "ERROR:SHADER:LINKING" << std::endl << infoLog << std::endl;
        delete []infoLog;
        exit(1);
    }

    glDeleteShader(vertexShader);
    if (hasGShader) glDeleteShader(geometryShader);
    glDeleteShader(fragmentShader);

    std::cout << "Shader program created" << std::endl;
}

Shader::~Shader() {
    glDetachShader(shaderID, vertexShader);
    if (geometryShader != 0) glDetachShader(shaderID, geometryShader);
    glDetachShader(shaderID, fragmentShader);

    glDeleteShader(vertexShader);
    if (geometryShader != 0) glDeleteShader(geometryShader);
    glDeleteShader(fragmentShader);

    glDeleteProgram(shaderID);
}

void Shader::use() const {
    glUseProgram(shaderID);
}

GLint Shader::a(const std::string& attribName) const {
    return glGetAttribLocation(shaderID, attribName.c_str());
}

GLint Shader::u(const std::string& uniformName) const {
    return glGetUniformLocation(shaderID, uniformName.c_str());
}

void Shader::setUniform(const std::string& name, const int value) const {
    glUniform1i(glGetUniformLocation(shaderID, name.c_str()), value);
}

void Shader::setUniform(const std::string& name, const float value) const {
    glUniform1f(glGetUniformLocation(shaderID, name.c_str()), value);
}

void Shader::setUniform(const std::string& name, const glm::vec3& value) const {
    glUniform3fv(glGetUniformLocation(shaderID, name.c_str()), 1, glm::value_ptr(value));
}

void Shader::setUniform(const std::string& name, const glm::vec4& value) const {
    glUniform4fv(glGetUniformLocation(shaderID, name.c_str()), 1, glm::value_ptr(value));
}

void Shader::setUniform(const std::string& name, const glm::mat4& value, const bool transpose) const {
    glUniformMatrix4fv(glGetUniformLocation(shaderID, name.c_str()), 1, transpose, glm::value_ptr(value));
}
