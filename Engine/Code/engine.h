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
typedef glm::mat4x4  mat4;

class Camera;
class App;

const mat4 identityMatrix = mat4(
    1.0f, 0.0f, 0.0f, 0.0f,
    0.0f, 1.0f, 0.0f, 0.0f,
    0.0f, 0.0f, 1.0f, 0.0f,
    0.0f, 0.0f, 0.0f, 1.0f);

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
    //GLuint uniformBufferHandle;
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

struct Buffer 
{
    GLuint handle;
    GLenum type;
    u32 size;
    u32 head;
    void* data;
};

enum LightType
{
    LightType_Directional,
    LightType_Point,
};

struct Light
{
    LightType type;
    vec3 color;
    vec3 direction;
    vec3 position;
};

struct Object
{
    mat4 worldMatrix;
    mat4 worldViewProjection;
    vec3 position = vec3(0.0f,0.0f,0.0f);
    vec3 rotation = vec3(0.0f, 0.0f, 0.0f);
    vec3 scale = vec3(1.0f, 1.0f, 1.0f);
    u32 modelIndex;
    u32 localParamsOffset;
    u32 localParamsSize;

    void UpdateWorldMatrix()
    {
        // Reset the world matrix
        worldMatrix = glm::mat4(1.0f); // Identity matrix
        // Apply transformations
        worldMatrix = glm::translate(worldMatrix, position);
        worldMatrix = glm::rotate(worldMatrix, rotation.x, vec3(1, 0, 0)); // Rotate around X axis
        worldMatrix = glm::rotate(worldMatrix, rotation.y, vec3(0, 1, 0)); // Rotate around Y axis
        worldMatrix = glm::rotate(worldMatrix, rotation.z, vec3(0, 0, 1)); // Rotate around Z axis
        worldMatrix = glm::scale(worldMatrix, scale);
    }

    void SetTransform(vec3 transform)
    {
        position = transform;
        UpdateWorldMatrix();
    }

    vec3 GetTransform()
    {
        return position;
    }

    void SetRotation(vec3 new_rotation)
    {
        rotation = new_rotation;
        UpdateWorldMatrix();
    }

    vec3 GetRotation()
    {
        return rotation;
    }

    void SetScale(vec3 new_scale)
    {
        scale = new_scale;
        UpdateWorldMatrix();
    }

    vec3 GetScale()
    {
        return scale;
    }
};

enum Mode
{
    //Mode_TexturedQuad,
    //Mode_AlbedoPatrick,
    //Mode_Count

    //Mode_FinalRender,
    //Mode_Diffuse,
    //Mode_Specular,
    //Mode_Albedo,
    //Mode_Normal,

    Mode_ForwardRendering,
    Mode_DeferredRendering
};


class Camera
{
public:
    float speed;
    float sensitivity;
    vec3 X, Y, Z, Position, currentReference;
    mat4 ViewMatrix, ViewMatrixInverse;
    float zNear;
    float zFar;
    float fov;



    void SetValues();

    void CalculateViewMatrix();

    void UpdateCamera(App* app);
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
    u32 finalRender;
    u32 diffuse;
    u32 specular;
    u32 albedo;
    u32 normal;
    u32 lightDeferred;// Create a deferred light program
    u32 geoDeferred; // Create a Deferred geo program
    u32 renderLeQuad;

    GLuint quadVAO = 0;
    Buffer quadVBO;
    Buffer elementBuffer;

    // Other indices
    u32 programToRenderForward = 0;


    // texture indices
    u32 diceTexIdx;
    u32 whiteTexIdx;
    u32 blackTexIdx;
    u32 normalTexIdx;
    u32 magentaTexIdx;
    //u32 modelIdx;

    // MRTs
    GLuint gPosition, gNormal, gAlbedoSpec; // Texture handles for G-buffer
    Buffer gbuffer; // Framebuffer handle for G-buffer

    //u32 gPosition;


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
    Camera camera;

    //Lights
    std::vector<Light> lights;

    // Buffer
    Buffer cbuffer;
    u32 globalParamsOffset;
    u32 globalParamsSize;

    std::vector<Texture> textures;
    std::vector<Material>  materials;
    std::vector<Mesh>  meshes;
    std::vector<Model>  models;
    std::vector<Program>  programs;

    std::vector<Object> objects;


    int maxUniformBufferSize;
    int uniformBlockAlignment;

    
};

void Init(App* app);

void Gui(App* app);

void Update(App* app);

void Render(App* app);




