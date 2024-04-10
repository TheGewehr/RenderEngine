//
// engine.h: This file contains the types and functions relative to the engine.
//

#pragma once

#include "platform.h"
#include <glad/glad.h>

typedef glm::vec2  vec2;
typedef glm::vec3  vec3;
typedef glm::vec4  vec4;
typedef glm::ivec2 ivec2;
typedef glm::ivec3 ivec3;
typedef glm::ivec4 ivec4;
typedef glm::mat4  mat4;

///////////////////
// Quad with texture structs
struct VertexV3V2
{
    glm::vec3 pos;
    glm::vec2 uv;
};

const VertexV3V2 vertices[] = {
     {glm::vec3(-0.5,-0.5,0.0),glm::vec2(0.0,0.0)},
     {glm::vec3(0.5,-0.5,0.0),glm::vec2(1.0,0.0)},
     {glm::vec3(0.5,0.5,0.0),glm::vec2(1.0,1.0)},
     {glm::vec3(-0.5,0.5,0.0),glm::vec2(0.0,1.0)},
};

const u16 indices[] = {
    0,1,2,
    0,2,3
};
///////////////////////////////////////////
// VBO attributes
struct VertexBufferAttribute
{
    u8 location;
    u8 componentCount;
    u8 offset;
};

struct VertexBufferLayout
{
    std::vector<VertexBufferAttribute> attributes;
    u8 stride;
};

// Shader Attributes

struct VertexShaderAttribute
{
    u8 location;
    u8 componentCount;
};

struct VertexShaderLayout
{
    std::vector<VertexShaderAttribute> attributes;
};

// VAO, relating VBO with the shader
struct Vao
{
    GLuint handle;
    GLuint programHandle;
};
//////////////////////////////////////

struct Image
{
    void* pixels;
    ivec2 size;
    i32   nchannels;
    i32   stride;
};

struct Texture
{
    GLuint      handle;
    std::string filepath;
};

struct Model
{
    u32 meshIdx;
    std::vector<u32> materialIdx;
};

struct Submesh
{
    VertexBufferLayout vertexBufferLayout;
    std::vector<float> vertices;
    std::vector<u32> indices;
    u32 vertexOffset;
    u32 indexOffset;

    std::vector<Vao> vaos;
};

struct Mesh
{
    std::vector<Submesh> submeshes;
    GLuint vertexBufferHandle;
    GLuint indexBufferHandle;
};

struct Material
{
    std::string name;
    vec3 albedo;
    vec3 emissive;
    f32 smoothness;
    u32 albedoTextureIdx;
    u32 emissiveTextureIdx;
    u32 specularTextureIdx;
    u32 normalsTextureIdx;
    u32 bumpTextureIdx;
};

struct Program
{
    GLuint             handle;
    std::string        filepath;
    std::string        programName;
    u64                lastWriteTimestamp;

    VertexShaderLayout vertexInputLayout;
};

enum Camera_Movement {
    CAMERA_FORWARD,
    CAMERA_BACKWARD,
    CAMERA_LEFT,
    CAMERA_RIGHT
};

class Camera
{
private:

    // Default camera values
    const float YAW = -90.0f;
    const float PITCH = 0.0f;
    const float SPEED = 2.5f;
    const float SENSITIVITY = 0.1f;
    const float ZOOM = 45.0f;

public:
    Camera();
    Camera(glm::vec3 position, glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f), float yaw = -90.0f, float pitch = 0.0f);

    vec3 position;
    vec3 front;
    vec3 up;
    vec3 right;
    vec3 worldUp;

    float yaw = -90.0f;
    float pitch = 0.0f;

    float movementSpeed = 0.5f;
    float mouseSensitivity = 0.5f;
    float zoom = 0.5f;

    mat4 GetViewMatrix();
    void ProcessKeyboard(Camera_Movement direction, float deltaTime);
    void ProcessMouseMovement(float xoffset, float yoffset, GLboolean constrainPitch = true);
    void ProcessMouseScroll(float yoffset);

    void UpdateCameraVectors();
};

struct Object
{
    mat4 worldMatrix;
    mat4 worldViewProjection;
    vec3 position;
    u32 modelIndex;
    //u32 localParamsOffset;
    //u32 localParamsSize;
};

enum Mode
{
    Mode_TexturedQuad,
    Mode_AlbedoPatrick,
    Mode_Count
};


struct App
{
    // Loop
    f32  deltaTime;
    bool isRunning;

    // Input
    Input input;

    // Graphics
    char gpuName[64];
    char openGlVersion[64];

    ivec2 displaySize;

    // program indices
    u32 texturedMeshProgramIdx;

    // texture indices
    u32 diceTexIdx;
    u32 whiteTexIdx;
    u32 blackTexIdx;
    u32 normalTexIdx;
    u32 magentaTexIdx;
    //u32 modelIdx;


    // Mode
    Mode mode;

    // Embedded geometry (in-editor simple meshes such as
    // a screen filling quad, a cube, a sphere...)
    GLuint embeddedVertices;
    GLuint embeddedElements;

    // Location of the texture uniform in the textured quad shader
    GLuint programUniformTexture;

    // VAO object to link our screen filling quad with our textured quad shader
    GLuint vao;

    // Camera
    Camera camera = Camera(vec3(0.25f, 1.25f, 6.75f));

    std::vector<Texture> textures;
    std::vector<Material>  materials;
    std::vector<Mesh>  meshes;
    std::vector<Model>  models;
    std::vector<Program>  programs;

    std::vector<Object> objects;

    
};

void Init(App* app);

void Gui(App* app);

void Update(App* app);

void Render(App* app);





