#pragma once
#define STB_IMAGE_IMPLEMENTATION
#include "shader.hpp"
#include <iostream>

namespace SkyboxData {
    inline float vertices[] = {
        -1.0f, 1.0f, -1.0f, -1.0f, -1.0f, -1.0f, 1.0f, -1.0f, -1.0f,
        1.0f, -1.0f, -1.0f, 1.0f, 1.0f, -1.0f, -1.0f, 1.0f, -1.0f,

        -1.0f, -1.0f, 1.0f, -1.0f, -1.0f, -1.0f, -1.0f, 1.0f, -1.0f,
        -1.0f, 1.0f, -1.0f, -1.0f, 1.0f, 1.0f, -1.0f, -1.0f, 1.0f,

        1.0f, -1.0f, -1.0f, 1.0f, -1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, 1.0f, 1.0f, 1.0f, -1.0f, 1.0f, -1.0f, -1.0f,

        -1.0f, -1.0f, 1.0f, -1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, 1.0f, 1.0f, -1.0f, 1.0f, -1.0f, -1.0f, 1.0f,

        -1.0f, 1.0f, -1.0f, 1.0f, 1.0f, -1.0f, 1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, 1.0f, -1.0f, 1.0f, 1.0f, -1.0f, 1.0f, -1.0f,

        -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, 1.0f, 1.0f, -1.0f, -1.0f,
        1.0f, -1.0f, -1.0f, -1.0f, -1.0f, 1.0f, 1.0f, -1.0f, 1.0f
    };

    inline unsigned int indices[] = {
        0, 1, 2, 2, 3, 0, // back
        4, 5, 6, 6, 7, 4, // front
        4, 5, 1, 1, 0, 4, // left
        3, 2, 6, 6, 7, 3, // right
        4, 0, 3, 3, 7, 4, // top
        1, 5, 6, 6, 2, 1
    }; //bottom

    inline std::string facesCubemap[6] = {
        "right.png",
        "left.png",
        "top.png",
        "bottom.png",
        "front.png",
        "back.png"
    };
}

class Skybox {
    inline static Shader *skyboxShader;

    inline static unsigned int cubemapTexture;
    inline static unsigned int skyboxVAO, skyboxVBO, skyboxEBO;

    static void loadTextures() {
        stbi_set_flip_vertically_on_load(false);
        for (unsigned int i = 0; i < 6; i++) {
            int width, height, nrChannels;
            const auto data = stbi_load((std::string(SKYBOX_PATH) + SkyboxData::facesCubemap[i]).c_str(), &width,
                                        &height,
                                        &nrChannels,
                                        0);
            if (data) {
                glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE,
                             data);
                stbi_image_free(data);
            } else {
                std::cerr << "Failed to load sky texture: " << stbi_failure_reason() << std::endl;
                stbi_image_free(data);
            }
        }
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

        glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
    }

public:
    Skybox() = delete;

    static void init() {
        skyboxShader = new Shader("skybox_vert.glsl", nullptr, "skybox_frag.glsl");

        glGenVertexArrays(1, &skyboxVAO);
        glBindVertexArray(skyboxVAO);

        // Setup VBO
        glGenBuffers(1, &skyboxVBO);
        glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(SkyboxData::vertices), SkyboxData::vertices, GL_STATIC_DRAW);

        // Setup EBO
        glGenBuffers(1, &skyboxEBO);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, skyboxEBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(SkyboxData::indices), SkyboxData::indices,
                     GL_STATIC_DRAW);

        // Vertex attribute pointer
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), nullptr);
        glEnableVertexAttribArray(0);

        // Unbind
        glBindVertexArray(0);

        glGenTextures(1, &cubemapTexture);

        glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
        loadTextures();
    }

    static void draw(const glm::mat4 &viewMatrix, const glm::mat4 &projectionMatrix) {
        glDepthMask(GL_FALSE);
        glDepthFunc(GL_LEQUAL);

        skyboxShader->use();
        skyboxShader->setUniform("skybox", 0);

        /* Strip the translation out of view matrix so the skybox is locked at initial position */
        /* Same as viewMatrix[3] = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f) */
        skyboxShader->setUniform("V", glm::mat4(glm::mat3(viewMatrix)));
        skyboxShader->setUniform("P", projectionMatrix);

        glBindVertexArray(skyboxVAO);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
        skyboxShader->setUniform("skybox", 0);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glBindVertexArray(0);
        glDepthMask(GL_TRUE);
        glDepthFunc(GL_LESS);
    }
};



