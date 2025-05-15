#pragma once

#include <optional>
#include <vector>
#include <string>
#include <glm/glm.hpp>
#include "assimp/scene.h"
#include "assimp/postprocess.h"
#include "shader.hpp"

constexpr GLuint DEFAULT_P_FLAGS = aiProcess_Triangulate | aiProcess_PreTransformVertices;

struct Vertex {
    glm::vec3 Position;
    glm::vec3 Normal;
    glm::vec2 TexCoords;
};

struct Texture {
    unsigned int id;
    std::string type;
    std::string path;
};

class Mesh {
public:
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    std::vector<Texture> textures;

    Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, std::vector<Texture> textures);
    void Draw(const Shader &shader) const;

private:
    unsigned int VAO{}, VBO{}, EBO{};
    void setupMesh();
};

class Model {
public:
    explicit Model(const std::string& path,
                   bool flipTexturesVertically = false,
                   unsigned int pFlags = DEFAULT_P_FLAGS);

    void Draw(Shader &shader);

private:
    std::vector<Mesh> meshes;
    std::string directory;
    std::vector<Texture> loadedTextures;

    void loadModel(const std::string& path, unsigned int pFlags);
    void processNode(const aiNode *node, const aiScene *scene);
    Mesh processMesh(const aiMesh *mesh, const aiScene *scene);
    std::vector<Texture> loadMaterialTextures(const aiMaterial *mat, const aiScene *scene, aiTextureType type, const std::string& typeName);
};

unsigned int TextureFromMemory(const unsigned char* dataBuffer, size_t dataSize, const std::string& nameHint = "");
unsigned int TextureFromFile(const char *path, const std::string &directory);
