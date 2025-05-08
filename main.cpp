#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <iostream>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include "shader_program.hpp"
#include "camera.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

ShaderProgram *sp;

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


unsigned int TextureFromFile(const char *path, const std::string &directory)
{
    std::string filename = std::string(path);
    filename = directory + '/' + filename;

    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    unsigned char *data = stbi_load(filename.c_str(), &width, &height, &nrComponents, 0);
    if (data)
    {
        GLenum format;
        if (nrComponents == 1)
            format = GL_RED;
        else if (nrComponents == 3)
            format = GL_RGB;
        else if (nrComponents == 4)
            format = GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    }
    else
    {
        std::cout << "Texture failed to load at path: " << path << std::endl;
        stbi_image_free(data);
    }

    return textureID;
}


class Mesh {
private:
    unsigned int VAO, VBO, EBO;
    void setupMesh();
public:
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    std::vector<Texture> textures;

    Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, std::vector<Texture> textures);
    void Draw(ShaderProgram &shader);
};

void Mesh::setupMesh()
{
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int),
                 &indices[0], GL_STATIC_DRAW);

    // vertex positions
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
    // vertex normals
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Normal));
    // vertex texture coords
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, TexCoords));

    glBindVertexArray(0);
}

Mesh::Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, std::vector<Texture> textures)
{
    this->vertices = vertices;
    this->indices = indices;
    this->textures = textures;

    setupMesh();
}

void Mesh::Draw(ShaderProgram &shader)
{
    unsigned int diffuseNr = 1;
    unsigned int specularNr = 1;
    for(unsigned int i = 0; i < textures.size(); i++)
    {
        glActiveTexture(GL_TEXTURE0 + i); // activate proper texture unit before binding
        // retrieve texture number (the N in diffuse_textureN)
        std::string number;
        std::string name = textures[i].type;
        if(name == "texture_diffuse")
            number = std::to_string(diffuseNr++);
        else if(name == "texture_specular")
            number = std::to_string(specularNr++);

        glUniform1i(sp->u(("material." + name + number).c_str()), i);
        glBindTexture(GL_TEXTURE_2D, textures[i].id);
    }

    glActiveTexture(GL_TEXTURE0);

    // draw mesh
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

class Model
{
public:
    explicit Model(std::string path)
    {
        loadModel(path);
    }
    void Draw(ShaderProgram &shader);
private:
    // model data
    std::vector<Mesh> meshes;
    std::string directory;
    std::vector<Texture> loadedTextures;

    void loadModel(std::string path);
    void processNode(aiNode *node, const aiScene *scene);
    Mesh processMesh(aiMesh *mesh, const aiScene *scene);
    std::vector<Texture> loadMaterialTextures(aiMaterial *mat, unsigned int materialIndex,
                                         std::string typeName);
};

void Model::Draw(ShaderProgram &shader) {
    for (unsigned int i = 0; i < meshes.size(); ++i)
        meshes[i].Draw(shader);
}

void Model::loadModel(std::string path) {
    Assimp::Importer import;

    std::cout << "Loading " << path << std::endl;

    const aiScene *scene = import.ReadFile(path,
        aiProcess_Triangulate | aiProcess_JoinIdenticalVertices | aiProcess_SortByPType);

    if(!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
    {
        std::cout << "ERROR::ASSIMP::" << import.GetErrorString() << std::endl;
        return;
    }
    directory = path.substr(0, path.find_last_of('/'));

    processNode(scene->mRootNode, scene);

    std::cout << "Model " << path << " loaded!" << std::endl;
}

void Model::processNode(aiNode *node, const aiScene *scene)
{
    // process all the node's meshes (if any)
    for(unsigned int i = 0; i < node->mNumMeshes; i++)
    {
        aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];
        meshes.push_back(processMesh(mesh, scene));
    }
    // then do the same for each of its children
    for(unsigned int i = 0; i < node->mNumChildren; i++)
    {
        processNode(node->mChildren[i], scene);
    }
}

Mesh Model::processMesh(aiMesh *mesh, const aiScene *scene)
{
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    std::vector<Texture> textures;

    for(unsigned int i = 0; i < mesh->mNumVertices; i++)
    {
        Vertex vertex;
        // process vertex positions, normals and texture coordinates

        glm::vec3 vector;
        vector.x = mesh->mVertices[i].x;
        vector.y = mesh->mVertices[i].y;
        vector.z = mesh->mVertices[i].z;
        vertex.Position = vector;

        vector.x = mesh->mNormals[i].x;
        vector.y = mesh->mNormals[i].y;
        vector.z = mesh->mNormals[i].z;
        vertex.Normal = vector;

        if(mesh->mTextureCoords[0]) // does the mesh contain texture coordinates?
        {
            glm::vec2 vec;
            vec.x = mesh->mTextureCoords[0][i].x;
            vec.y = mesh->mTextureCoords[0][i].y;
            vertex.TexCoords = vec;
        }
        else
            vertex.TexCoords = glm::vec2(0.0f, 0.0f);

        vertices.push_back(vertex);
    }

    // process indices
    for(unsigned int i = 0; i < mesh->mNumFaces; i++)
    {
        aiFace face = mesh->mFaces[i];
        for(unsigned int j = 0; j < face.mNumIndices; j++)
            indices.push_back(face.mIndices[j]);
    }

    // process material
    std::vector<Texture> indexTextures = loadMaterialTextures(
        scene->mMaterials[mesh->mMaterialIndex],
        mesh->mMaterialIndex,
        "texture_index"
    );
    textures.insert(textures.end(), indexTextures.begin(), indexTextures.end());

    // if(mesh->mMaterialIndex >= 0)
    // {
    //     aiMaterial *material = scene->mMaterials[mesh->mMaterialIndex];
    //     std::vector<Texture> diffuseMaps = loadMaterialTextures(material,
    //                                         aiTextureType_DIFFUSE, "texture_diffuse");
    //     textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
    //     std::vector<Texture> specularMaps = loadMaterialTextures(material,
    //                                         aiTextureType_SPECULAR, "texture_specular");
    //     textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
    // }

    return Mesh(vertices, indices, textures);
}

std::vector<Texture> Model::loadMaterialTextures(aiMaterial *mat, unsigned int materialIndex, std::string typeName)
{
    std::vector<Texture> textures;

    // Construct the filename from the material index
    std::string filename = "gltf_embedded_" + std::to_string(materialIndex) + ".png";

    // Check if already loaded
    for (const auto& tex : loadedTextures) {
        if (tex.path == filename) {
            textures.push_back(tex);
            return textures;
        }
    }

    Texture texture;
    texture.id = TextureFromFile(filename.c_str(), directory);
    texture.type = typeName;
    texture.path = filename;

    textures.push_back(texture);
    loadedTextures.push_back(texture);

    return textures;
}


// std::vector<Texture> Model::loadMaterialTextures(aiMaterial *mat, aiTextureType type, std::string typeName)
// {
//     std::vector<Texture> textures;
//     for(unsigned int i = 0; i < mat->GetTextureCount(type); i++)
//     {
//         aiString str;
//         mat->GetTexture(type, i, &str);
//         bool skip = false;
//         for(unsigned int j = 0; j < loadedTextures.size(); j++)
//         {
//             if(std::strcmp(loadedTextures[j].path.data(), str.C_Str()) == 0)
//             {
//                 textures.push_back(loadedTextures[j]);
//                 skip = true;
//                 break;
//             }
//         }
//         if(!skip)
//         {   // if texture hasn't been loaded already, load it
//             Texture texture;
//             texture.id = TextureFromFile(str.C_Str(), directory);
//             texture.type = typeName;
//             texture.path = str.C_Str();
//             textures.push_back(texture);
//             loadedTextures.push_back(texture); // add to loaded textures
//         }
//     }
//     return textures;
// }

void testAssimpImport() {
    Assimp::Importer importer;

    const std::string path = "../models/track.obj";

    const aiScene* scene = importer.ReadFile(path,
        aiProcess_Triangulate | aiProcess_JoinIdenticalVertices | aiProcess_SortByPType |
        aiProcess_GenSmoothNormals | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        std::cerr << "Assimp error: " << importer.GetErrorString() << std::endl;
        return;
    }

    std::cout << "Model loaded successfully." << std::endl;
    std::cout << "Number of meshes: " << scene->mNumMeshes << std::endl;
    std::cout << "Number of materials:" << scene->mNumMaterials << std::endl;
    std::cout << "Name of meshes:" << std::endl;

    for (unsigned int i = 0; i < scene->mNumMeshes; ++i) {
        aiMesh* mesh = scene->mMeshes[i];
        std::cout << "Mesh " << i << " has " << mesh->mNumVertices << " vertices." << std::endl;
    }
}

void errorCallback(int error, const char* description) {
    fputs(description, stderr);
}

float vertices[] = {
     0.5f,  0.5f, 0.0f,  // top right
     0.5f, -0.5f, 0.0f,  // bottom right
    -0.5f, -0.5f, 0.0f,  // bottom left
    -0.5f,  0.5f, 0.0f   // top left
};

unsigned int indices[] = {  // note that we start from 0!
    0, 1, 3,   // first triangle
    1, 2, 3    // second triangle
};

Camera camera(glm::vec3(0.0f, 20.0f, 3.0f));
Model *backpackModel;
float lastX = 800.0f / 2.0f;
float lastY = 600.0f / 2.0f;
bool firstMouse = true;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

void processInput(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, deltaTime);
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}

// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow* window, double xposIn, double yposIn)
{
    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);

    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

    lastX = xpos;
    lastY = ypos;

    camera.ProcessMouseMovement(xoffset, yoffset);
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(static_cast<float>(yoffset));
}

void drawScene(GLFWwindow *window) {
    float currentFrame = static_cast<float>(glfwGetTime());
    deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;

    processInput(window);

    glClearColor(0.05f, 0.05f, 0.05f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // don't forget to enable shader before setting uniforms
    sp->use();

    // view/projection transformations
    glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), 800.0f / 600.0f, 0.1f, 1000.0f);
    glm::mat4 view = camera.GetViewMatrix();

    glUniformMatrix4fv(sp->u("P"), 1, GL_FALSE, &projection[0][0]);
    glUniformMatrix4fv(sp->u("V"), 1, GL_FALSE, &view[0][0]);

    // render the loaded model
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f)); // translate it down so it's at the center of the scene
    model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));	// it's a bit too big for our scene, so scale it down
    glUniformMatrix4fv(sp->u("M"), 1, GL_FALSE, &model[0][0]);
    backpackModel->Draw(*sp);
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
    GLFWwindow* window = glfwCreateWindow(800, 600, "NFS PUT", nullptr, nullptr);
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

    // stbi_set_flip_vertically_on_load(true);
    glEnable(GL_DEPTH_TEST);

    backpackModel = new Model("../models/track.obj");
    sp = new ShaderProgram("../shaders/v_simplest.glsl", NULL, "../shaders/f_simplest.glsl");

    // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    // glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    while (!glfwWindowShouldClose(window)) {
        drawScene(window);
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwDestroyWindow(window);
    glfwTerminate();

    delete backpackModel;

    exit(EXIT_SUCCESS);
}
