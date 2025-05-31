#include "model.hpp"
#include <iostream>
#include "stb_image.h"
#include <cstring>
#include "assimp/Importer.hpp"
#include "assimp/postprocess.h"
#include "glm/gtc/type_ptr.inl"

unsigned int UploadTexture(const unsigned char *data, const int width, const int height, const int nrComponents) {
    if (!data) return 0;

    GLenum format;
    switch (nrComponents) {
        case 1:
            format = GL_RED;
            break;
        case 2:
            format = GL_RG;
            break;
        case 3:
            format = GL_RGB;
            break;
        case 4:
            format = GL_RGBA;
            break;
        default:
            std::cerr << "Unsupported number of components: " << nrComponents << std::endl;
            return 0;
    }

    unsigned int textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexImage2D(GL_TEXTURE_2D, 0, static_cast<GLint>(format), width, height, 0, format, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    return textureID;
}

unsigned int TextureFromMemory(const unsigned char *dataBuffer, const size_t dataSize, const std::string &nameHint) {
    int            width, height, nrComponents;
    unsigned char *data =
        stbi_load_from_memory(dataBuffer, static_cast<int>(dataSize), &width, &height, &nrComponents, 0);

    if (!data) {
        std::cerr << "Embedded texture failed to load";
        if (!nameHint.empty()) std::cerr << " (" << nameHint << ")";
        std::cerr << std::endl;
        return 0;
    }

    const unsigned int texID = UploadTexture(data, width, height, nrComponents);
    stbi_image_free(data);
    return texID;
}

unsigned int TextureFromFile(const char *path, const std::string &directory) {
    const std::string filename = directory + '/' + path;

    int            width, height, nrComponents;
    unsigned char *data = stbi_load(filename.c_str(), &width, &height, &nrComponents, 0);

    if (!data) {
        std::cerr << "Texture failed to load at path: " << filename << std::endl;
        return 0;
    }

    const unsigned int texID = UploadTexture(data, width, height, nrComponents);
    stbi_image_free(data);
    return texID;
}

Mesh::Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, std::vector<Texture> textures,
           const unsigned int materialID)
    : vertices(std::move(vertices)), indices(std::move(indices)), textures(std::move(textures)),
      materialID(materialID) {
    setupMesh();
}

void Mesh::setupMesh() {
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, static_cast<long>(vertices.size() * sizeof(Vertex)), &vertices[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, static_cast<long>(indices.size() * sizeof(unsigned int)), &indices[0],
                 GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *) 0);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *) offsetof(Vertex, Normal));

    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *) offsetof(Vertex, TexCoords));

    glBindVertexArray(0);
}

void Mesh::Draw(const Shader &shader) const {
    unsigned int diffuseNr = 1;
    unsigned int specularNr = 1;
    unsigned int baseColorNr = 1;
    unsigned int normalNr = 1;
    unsigned int roughnessNr = 1;
    unsigned int metalnessNr = 1;

    for (unsigned int i = 0; i < textures.size(); ++i) {
        glActiveTexture(GL_TEXTURE0 + i);
        std::string number;
        std::string name = textures[i].type;

        if (name == "texture_diffuse")
            number = std::to_string(diffuseNr++);
        else if (name == "texture_specular")
            number = std::to_string(specularNr++);
        else if (name == "texture_basecolor")
            number = std::to_string(baseColorNr++);
        else if (name == "texture_normal")
            number = std::to_string(normalNr++);
        else if (name == "texture_roughness")
            number = std::to_string(roughnessNr++);
        else if (name == "texture_metalness")
            number = std::to_string(metalnessNr++);

        glUniform1i(shader.u(name + number), static_cast<GLint>(i));
        glBindTexture(GL_TEXTURE_2D, textures[i].id);
    }

    if (diffuseNr == 1 && baseColorNr == 1) {
        glUniform4fv(shader.u("u_baseColor"), 1, glm::value_ptr(baseColor));
        glUniform1i(shader.u("u_hasBaseColorTexture"), false);
    } else {
        glUniform1i(shader.u("u_hasBaseColorTexture"), true);
    }

    glUniform1ui(shader.u("u_materialID"), materialID);
    glActiveTexture(GL_TEXTURE0);
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, nullptr);
    glBindVertexArray(0);
}

Model::Model(const std::string &path, const bool flipTexturesVertically, const unsigned int pFlags) {
    stbi_set_flip_vertically_on_load(flipTexturesVertically);
    loadModel(path, pFlags);
}

void Model::Draw(Shader &shader) {
    for (auto &mesh : meshes)
        mesh.Draw(shader);
}

std::vector<Mesh> Model::getMeshes() {
    return meshes;
}

void Model::loadModel(const std::string &path, unsigned int pFlags) {
    std::cout << "Loading model " << path << std::endl;

    Assimp::Importer import;
    const aiScene   *scene =
        import.ReadFile(std::string(MODELS_PATH) + path, aiProcess_Triangulate | aiProcess_PreTransformVertices);

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        std::cout << "ERROR::ASSIMP::" << import.GetErrorString() << std::endl;
        return;
    }
    directory = path.substr(0, path.find_last_of('/'));
    processNode(scene->mRootNode, scene);

    std::cout << "Finished loading model " << path << std::endl;
}

void Model::processNode(const aiNode *node, const aiScene *scene) {
    for (unsigned int i = 0; i < node->mNumMeshes; i++) {
        aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];
        meshes.push_back(processMesh(mesh, scene));
    }
    for (unsigned int i = 0; i < node->mNumChildren; i++) {
        processNode(node->mChildren[i], scene);
    }
}

Mesh Model::processMesh(const aiMesh *mesh, const aiScene *scene) {
    std::vector<Vertex>       vertices;
    std::vector<unsigned int> indices;
    std::vector<Texture>      textures;

    for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
        Vertex vertex{};
        vertex.Position = glm::vec3(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z);
        vertex.Normal = glm::vec3(mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z);

        if (mesh->mTextureCoords[0])
            vertex.TexCoords = glm::vec2(mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y);
        else
            vertex.TexCoords = glm::vec2(0.0f, 0.0f);

        vertices.push_back(vertex);
    }

    for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
        for (unsigned int j = 0; j < mesh->mFaces[i].mNumIndices; j++)
            indices.push_back(mesh->mFaces[i].mIndices[j]);
    }

    std::cout << "material id: " << mesh->mMaterialIndex << std::endl;
    const aiMaterial *material = scene->mMaterials[mesh->mMaterialIndex];

    /* Debug */
    constexpr aiTextureType textureTypes[] = {
        aiTextureType_DIFFUSE, aiTextureType_BASE_COLOR, aiTextureType_METALNESS, aiTextureType_NORMALS,
        aiTextureType_DIFFUSE_ROUGHNESS
    };

    for (const auto textureType: textureTypes) {
        if (material->GetTextureCount(textureType) > 0)
            std::cout << "Found type of texture with id " << textureType << std::endl;
    }

    auto diffuseMaps = loadMaterialTextures(material, scene, aiTextureType_DIFFUSE, "texture_diffuse");
    textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
    auto baseColorMaps = loadMaterialTextures(material, scene, aiTextureType_BASE_COLOR, "texture_basecolor");
    textures.insert(textures.end(), baseColorMaps.begin(), baseColorMaps.end());

    auto specularMaps = loadMaterialTextures(material, scene, aiTextureType_SPECULAR, "texture_specular");
    textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());

    auto metalnessMaps = loadMaterialTextures(material, scene, aiTextureType_METALNESS, "texture_metalness");
    textures.insert(textures.end(), metalnessMaps.begin(), metalnessMaps.end());

    auto roughnessMaps = loadMaterialTextures(material, scene, aiTextureType_DIFFUSE_ROUGHNESS, "texture_roughness");
    textures.insert(textures.end(), roughnessMaps.begin(), roughnessMaps.end());

    auto normalMaps = loadMaterialTextures(material, scene, aiTextureType_NORMALS, "texture_normal");
    textures.insert(textures.end(), normalMaps.begin(), normalMaps.end());

    auto newMesh = new Mesh(vertices, indices, textures, mesh->mMaterialIndex);
    if (material->GetTextureCount(aiTextureType_DIFFUSE) == 0 &&
        material->GetTextureCount(aiTextureType_BASE_COLOR) == 0) {
        aiColor4D color;
        if (AI_SUCCESS == material->Get(AI_MATKEY_BASE_COLOR, color)) {
            newMesh->baseColor = glm::vec4(color.r, color.g, color.b, color.a);
        } else {
            newMesh->baseColor = glm::vec4(1.0);
        }
    }
    return *newMesh;
}

std::vector<Texture> Model::loadMaterialTextures(const aiMaterial *mat, const aiScene *scene, const aiTextureType type,
                                                 const std::string &typeName) {
    std::vector<Texture> textures;
    for (unsigned int i = 0; i < mat->GetTextureCount(type); i++) {
        aiString str;
        aiTextureMapping uvIndex;
        mat->GetTexture(type, i, &str, &uvIndex);

        if (uvIndex != 0) {
            std::cout << "uvIndex is " << uvIndex << std::endl;
        }

        auto it = std::find_if(loadedTextures.begin(), loadedTextures.end(),
                               [&](const Texture &t) { return std::strcmp(t.path.c_str(), str.C_Str()) == 0; });

        if (it != loadedTextures.end()) {
            textures.push_back(*it);
            continue;
        }

        Texture texture;
        if (str.C_Str()[0] == '*') {
            const int  textureIndex = std::stoi(str.C_Str() + 1);
            aiTexture *embeddedTexture = scene->mTextures[textureIndex];

            std::cout << str.C_Str() << std::endl;
            if (embeddedTexture->mHeight == 0) {
                texture.id = TextureFromMemory(reinterpret_cast<unsigned char *>(embeddedTexture->pcData),
                                               embeddedTexture->mWidth, str.C_Str());
            } else {
                std::cout << "Tried to load unknown texture format (uncompressed): " << str.C_Str() << std::endl;
                continue;
            }
        } else {
            texture.id = TextureFromFile(str.C_Str(), directory);
        }

        texture.type = typeName;
        texture.path = str.C_Str();
        textures.push_back(texture);
        loadedTextures.push_back(texture);
    }

    return textures;
}