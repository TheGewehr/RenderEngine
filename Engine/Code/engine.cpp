//
// engine.cpp : Put all your graphics stuff in this file. This is kind of the graphics module.
// In here, you should type all your OpenGL commands, and you can also type code to handle
// input platform events (e.g to move the camera or react to certain shortcuts), writing some
// graphics related GUI options, and so on.
//
#define PI 3.1416f
#define BINDING(b) b

#include "engine.h"
#include <imgui.h>
#include <stb_image.h>
#include <stb_image_write.h>
#include <assimp/cimport.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <iostream>

void OnGlError(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam)
{
    if (severity == GL_DEBUG_SEVERITY_NOTIFICATION)
        return;

    ELOG("OpenGL debug message: %s", message);

    switch (source)
    {
    case GL_DEBUG_SOURCE_API:               ELOG(" - source: GL_DEBUG_SOURCE_API"); break; // Calls to the OpenGL API
    case GL_DEBUG_SOURCE_WINDOW_SYSTEM:     ELOG(" - source: GL_DEBUG_SOURCE_WINDOW_SYSTEM"); break; // Calls to a window-system API
    case GL_DEBUG_SOURCE_SHADER_COMPILER:   ELOG(" - source: GL_DEBUG_SOURCE_SHADER_COMPILER"); break; // A compiler for shading language
    case GL_DEBUG_SOURCE_THIRD_PARTY:       ELOG(" - source: GL_DEBUG_SOURCE_THIRD_PARTY"); break; // An application associated with OpenGl
    case GL_DEBUG_SOURCE_APPLICATION:       ELOG(" - source: GL_DEBUG_SOURCE_APPLICATION"); break;  // Generated by the user of this application
    case GL_DEBUG_SOURCE_OTHER:             ELOG(" - source: GL_DEBUG_SOURCE_OTHER"); break; // Some source that isn't one of these
    }

    switch (type)
    {
    case GL_DEBUG_TYPE_ERROR:               ELOG(" - type: GL_DEBUG_TYPE_ERROR"); break; // An error, typically from the APi
    case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: ELOG(" - type: GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR"); break; // Some behavior marked deprecated
    case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:  ELOG(" - type: GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR"); break; // Something has invoked undefined behavior
    case GL_DEBUG_TYPE_PORTABILITY:         ELOG(" - type: GL_DEBUG_TYPE_PORTABILITY"); break; // Some functionality the user relies upon
    case GL_DEBUG_TYPE_PERFORMANCE:         ELOG(" - type: GL_DEBUG_TYPE_PERFORMANCE"); break; // Code has triggered possible performance issues
    case GL_DEBUG_TYPE_MARKER:              ELOG(" - type: GL_DEBUG_TYPE_MARKER"); break; // Command stream annotation
    case GL_DEBUG_TYPE_PUSH_GROUP:          ELOG(" - type: GL_DEBUG_TYPE_PUSH_GROUP"); break; // Group pushing
    case GL_DEBUG_TYPE_POP_GROUP:           ELOG(" - type: GL_DEBUG_TYPE_POP_GROUP"); break; // foo
    case GL_DEBUG_TYPE_OTHER:               ELOG(" - type: GL_DEBUG_TYPE_OTHER"); break; // Some type that isn't one of these
    }

    switch (severity)
    {
    case GL_DEBUG_SEVERITY_HIGH:            ELOG(" - severity: GL_DEBUG_SEVERITY_HIGH"); break; // All OpenGL Errors, shader compilations/link
    case GL_DEBUG_SEVERITY_MEDIUM:          ELOG(" - severity: GL_DEBUG_SEVERITY_MEDIUM"); break; // Major performance warnings, shader compilation
    case GL_DEBUG_SEVERITY_LOW:             ELOG(" - severity: GL_DEBUG_SEVERITY_LOW"); break; // Redundant state change performance warning
    case GL_DEBUG_SEVERITY_NOTIFICATION:    ELOG(" - severity: GL_DEBUG_SEVERITY_NOTIFICATION"); break; // Anything that isn't an error
    }
};

// Function to display glm::vec3
void logVec3(const glm::vec3& v) {
    std::cout << "glm::vec3: (" << v.x << ", " << v.y << ", " << v.z << ")" << std::endl;
}

// Function to display glm::mat4
void logMat4(const glm::mat4& m) {
    std::cout << "glm::mat4:" << std::endl;
    for (int i = 0; i < 4; i++) {
        std::cout << "(";
        for (int j = 0; j < 4; j++) {
            std::cout << m[i][j];
            if (j < 3) std::cout << ", ";
        }
        std::cout << ")" << std::endl;
    }
}

vec3 rotate(const vec3& vector, float degrees, const vec3& axis)
{
    float radians = glm::radians(degrees);

    glm::mat4 rotationMatrix = glm::rotate(glm::mat4(1.0f), radians, axis);

    glm::vec4 rotatedVector = rotationMatrix * glm::vec4(vector, 1.0f);

    return glm::vec3(rotatedVector);
}

u8 LoadProgramAttributes(Program& program)
{
    GLsizei attributeCount;
    glGetProgramiv(program.handle, GL_ACTIVE_ATTRIBUTES, &attributeCount);

    for (u32 i = 0; i < attributeCount; ++i)
    {
        GLchar attributeName[128];
        GLsizei attributeNameLenght;
        GLint attributeSize;
        GLenum attributeType;
        glGetActiveAttrib(program.handle, i, ARRAY_COUNT(attributeName), &attributeNameLenght, &attributeSize, &attributeType, attributeName);

        u8 attributeLoacation = glGetAttribLocation(program.handle, attributeName);
        program.vertexInputLayout.attributes.push_back({ attributeLoacation, (u8)attributeSize });
    }

    return attributeCount;
}

GLuint CreateProgramFromSource(String programSource, const char* shaderName)
{
    GLchar  infoLogBuffer[1024] = {};
    GLsizei infoLogBufferSize = sizeof(infoLogBuffer);
    GLsizei infoLogSize;
    GLint   success;

    char versionString[] = "#version 430\n";
    char shaderNameDefine[128];
    sprintf(shaderNameDefine, "#define %s\n", shaderName);
    char vertexShaderDefine[] = "#define VERTEX\n";
    char fragmentShaderDefine[] = "#define FRAGMENT\n";

    const GLchar* vertexShaderSource[] = {
        versionString,
        shaderNameDefine,
        vertexShaderDefine,
        programSource.str
    };
    const GLint vertexShaderLengths[] = {
        (GLint)strlen(versionString),
        (GLint)strlen(shaderNameDefine),
        (GLint)strlen(vertexShaderDefine),
        (GLint)programSource.len
    };
    const GLchar* fragmentShaderSource[] = {
        versionString,
        shaderNameDefine,
        fragmentShaderDefine,
        programSource.str
    };
    const GLint fragmentShaderLengths[] = {
        (GLint)strlen(versionString),
        (GLint)strlen(shaderNameDefine),
        (GLint)strlen(fragmentShaderDefine),
        (GLint)programSource.len
    };

    GLuint vshader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vshader, ARRAY_COUNT(vertexShaderSource), vertexShaderSource, vertexShaderLengths);
    glCompileShader(vshader);
    glGetShaderiv(vshader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(vshader, infoLogBufferSize, &infoLogSize, infoLogBuffer);
        ELOG("glCompileShader() failed with vertex shader %s\nReported message:\n%s\n", shaderName, infoLogBuffer);
    }

    GLuint fshader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fshader, ARRAY_COUNT(fragmentShaderSource), fragmentShaderSource, fragmentShaderLengths);
    glCompileShader(fshader);
    glGetShaderiv(fshader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(fshader, infoLogBufferSize, &infoLogSize, infoLogBuffer);
        ELOG("glCompileShader() failed with fragment shader %s\nReported message:\n%s\n", shaderName, infoLogBuffer);
    }

    GLuint programHandle = glCreateProgram();
    glAttachShader(programHandle, vshader);
    glAttachShader(programHandle, fshader);
    glLinkProgram(programHandle);
    glGetProgramiv(programHandle, GL_LINK_STATUS, &success);
    if (!success)
    {
        glGetProgramInfoLog(programHandle, infoLogBufferSize, &infoLogSize, infoLogBuffer);
        ELOG("glLinkProgram() failed with program %s\nReported message:\n%s\n", shaderName, infoLogBuffer);
    }

    glUseProgram(0);

    glDetachShader(programHandle, vshader);
    glDetachShader(programHandle, fshader);
    glDeleteShader(vshader);
    glDeleteShader(fshader);

    return programHandle;
}

u32 LoadProgram(App* app, const char* filepath, const char* programName)
{
    String programSource = ReadTextFile(filepath);

    Program program = {};
    program.handle = CreateProgramFromSource(programSource, programName);
    program.filepath = filepath;
    program.programName = programName;
    program.lastWriteTimestamp = GetFileLastWriteTimestamp(filepath);
    app->programs.push_back(program);

    return app->programs.size() - 1;
}

Image LoadImage(const char* filename)
{
    Image img = {};
    stbi_set_flip_vertically_on_load(true);
    img.pixels = stbi_load(filename, &img.size.x, &img.size.y, &img.nchannels, 0);
    if (img.pixels)
    {
        img.stride = img.size.x * img.nchannels;
    }
    else
    {
        ELOG("Could not open file %s", filename);
    }
    return img;
}

void FreeImage(Image image)
{
    stbi_image_free(image.pixels);
}

GLuint CreateTexture2DFromImage(Image image)
{
    GLenum internalFormat = GL_RGB8;
    GLenum dataFormat = GL_RGB;
    GLenum dataType = GL_UNSIGNED_BYTE;

    switch (image.nchannels)
    {
    case 3: dataFormat = GL_RGB; internalFormat = GL_RGB8; break;
    case 4: dataFormat = GL_RGBA; internalFormat = GL_RGBA8; break;
    default: ELOG("LoadTexture2D() - Unsupported number of channels");
    }

    GLuint texHandle;
    glGenTextures(1, &texHandle);
    glBindTexture(GL_TEXTURE_2D, texHandle);
    glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, image.size.x, image.size.y, 0, dataFormat, dataType, image.pixels);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glGenerateMipmap(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, 0);

    return texHandle;
}

u32 LoadTexture2D(App* app, const char* filepath)
{
    for (u32 texIdx = 0; texIdx < app->textures.size(); ++texIdx)
        if (app->textures[texIdx].filepath == filepath)
            return texIdx;

    Image image = LoadImage(filepath);

    if (image.pixels)
    {
        Texture tex = {};
        tex.handle = CreateTexture2DFromImage(image);
        tex.filepath = filepath;

        u32 texIdx = app->textures.size();
        app->textures.push_back(tex);

        FreeImage(image);
        return texIdx;
    }
    else
    {
        return UINT32_MAX;
    }
}

GLuint FindVAO(Mesh& mesh, u32 submeshIndex, const Program& program)
{
    Submesh& submesh = mesh.submeshes[submeshIndex];

    //Try finding a vao for this submesh/program
    for (u32 i = 0; i < (u32)submesh.vaos.size(); ++i)
    {
        if (submesh.vaos[i].programHandle == program.handle)
        {
            return submesh.vaos[i].handle;
        }
    }

    GLuint vaoHandle = 0;

    //Create a new vao for this submesh/program
    glGenVertexArrays(1, &vaoHandle);
    glBindVertexArray(vaoHandle);

    glBindBuffer(GL_ARRAY_BUFFER, mesh.vertexBufferHandle);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.indexBufferHandle);

    for (u32 i = 0; i < program.vertexInputLayout.attributes.size(); ++i)
    {
        bool attributeWasLinked = false;

        for (u32 j = 0; j < submesh.vertexBufferLayout.attributes.size(); ++j)
        {
            if (program.vertexInputLayout.attributes[i].location == submesh.vertexBufferLayout.attributes[j].location)
            {
                const u32 index = submesh.vertexBufferLayout.attributes[j].location;
                const u32 ncomp = submesh.vertexBufferLayout.attributes[j].componentCount;
                const u32 offset = submesh.vertexBufferLayout.attributes[j].offset + submesh.vertexOffset;  
                const u32 stride = submesh.vertexBufferLayout.stride;
                glVertexAttribPointer(index, ncomp, GL_FLOAT, GL_FALSE, stride, (void*)(u64)offset);
                glEnableVertexAttribArray(index);

                attributeWasLinked = true;
                break;
            }
        }        
    }

    glBindVertexArray(0);

    Vao vao = { vaoHandle, program.handle };
    submesh.vaos.push_back(vao);

    return vaoHandle;
}

void ProcessAssimpMesh(const aiScene* scene, aiMesh* mesh, Mesh* myMesh, u32 baseMeshMaterialIndex, std::vector<u32>& submeshMaterialIndices)
{
    std::vector<float> vertices;
    std::vector<u32> indices;

    bool hasTexCoords = false;
    bool hasTangentSpace = false;

    // process vertices
    for (unsigned int i = 0; i < mesh->mNumVertices; i++)
    {
        vertices.push_back(mesh->mVertices[i].x);
        vertices.push_back(mesh->mVertices[i].y);
        vertices.push_back(mesh->mVertices[i].z);
        vertices.push_back(mesh->mNormals[i].x);
        vertices.push_back(mesh->mNormals[i].y);
        vertices.push_back(mesh->mNormals[i].z);

        if (mesh->mTextureCoords[0]) // does the mesh contain texture coordinates?
        {
            hasTexCoords = true;
            vertices.push_back(mesh->mTextureCoords[0][i].x);
            vertices.push_back(mesh->mTextureCoords[0][i].y);
        }

        if (mesh->mTangents != nullptr && mesh->mBitangents)
        {
            hasTangentSpace = true;
            vertices.push_back(mesh->mTangents[i].x);
            vertices.push_back(mesh->mTangents[i].y);
            vertices.push_back(mesh->mTangents[i].z);

            // For some reason ASSIMP gives me the bitangents flipped.
            // Maybe it's my fault, but when I generate my own geometry
            // in other files (see the generation of standard assets)
            // and all the bitangents have the orientation I expect,
            // everything works ok.
            // I think that (even if the documentation says the opposite)
            // it returns a left-handed tangent space matrix.
            // SOLUTION: I invert the components of the bitangent here.
            vertices.push_back(-mesh->mBitangents[i].x);
            vertices.push_back(-mesh->mBitangents[i].y);
            vertices.push_back(-mesh->mBitangents[i].z);
        }
    }

    // process indices
    for (unsigned int i = 0; i < mesh->mNumFaces; i++)
    {
        aiFace face = mesh->mFaces[i];
        for (unsigned int j = 0; j < face.mNumIndices; j++)
        {
            indices.push_back(face.mIndices[j]);
        }
    }

    // store the proper (previously proceessed) material for this mesh
    submeshMaterialIndices.push_back(baseMeshMaterialIndex + mesh->mMaterialIndex);

    // create the vertex format
    VertexBufferLayout vertexBufferLayout = {};
    vertexBufferLayout.attributes.push_back(VertexBufferAttribute{ 0, 3, 0 });
    vertexBufferLayout.attributes.push_back(VertexBufferAttribute{ 1, 3, 3 * sizeof(float) });
    vertexBufferLayout.stride = 6 * sizeof(float);
    if (hasTexCoords)
    {
        vertexBufferLayout.attributes.push_back(VertexBufferAttribute{ 2, 2, vertexBufferLayout.stride });
        vertexBufferLayout.stride += 2 * sizeof(float);
    }
    if (hasTangentSpace)
    {
        vertexBufferLayout.attributes.push_back(VertexBufferAttribute{ 3, 3, vertexBufferLayout.stride });
        vertexBufferLayout.stride += 3 * sizeof(float);

        vertexBufferLayout.attributes.push_back(VertexBufferAttribute{ 4, 3, vertexBufferLayout.stride });
        vertexBufferLayout.stride += 3 * sizeof(float);
    }

    // add the submesh into the mesh
    Submesh submesh = {};
    submesh.vertexBufferLayout = vertexBufferLayout;
    submesh.vertices.swap(vertices);
    submesh.indices.swap(indices);
    myMesh->submeshes.push_back(submesh);
}

void ProcessAssimpMaterial(App* app, aiMaterial* material, Material& myMaterial, String directory)
{
    aiString name;
    aiColor3D diffuseColor;
    aiColor3D emissiveColor;
    aiColor3D specularColor;
    ai_real shininess;
    material->Get(AI_MATKEY_NAME, name);
    material->Get(AI_MATKEY_COLOR_DIFFUSE, diffuseColor);
    material->Get(AI_MATKEY_COLOR_EMISSIVE, emissiveColor);
    material->Get(AI_MATKEY_COLOR_SPECULAR, specularColor);
    material->Get(AI_MATKEY_SHININESS, shininess);

    myMaterial.name = name.C_Str();
    myMaterial.albedo = vec3(diffuseColor.r, diffuseColor.g, diffuseColor.b);
    myMaterial.emissive = vec3(emissiveColor.r, emissiveColor.g, emissiveColor.b);
    myMaterial.smoothness = shininess / 256.0f;

    aiString aiFilename;
    if (material->GetTextureCount(aiTextureType_DIFFUSE) > 0)
    {
        material->GetTexture(aiTextureType_DIFFUSE, 0, &aiFilename);
        String filename = MakeString(aiFilename.C_Str());
        String filepath = MakePath(directory, filename);
        myMaterial.albedoTextureIdx = LoadTexture2D(app, filepath.str);
    }
    if (material->GetTextureCount(aiTextureType_EMISSIVE) > 0)
    {
        material->GetTexture(aiTextureType_EMISSIVE, 0, &aiFilename);
        String filename = MakeString(aiFilename.C_Str());
        String filepath = MakePath(directory, filename);
        myMaterial.emissiveTextureIdx = LoadTexture2D(app, filepath.str);
    }
    if (material->GetTextureCount(aiTextureType_SPECULAR) > 0)
    {
        material->GetTexture(aiTextureType_SPECULAR, 0, &aiFilename);
        String filename = MakeString(aiFilename.C_Str());
        String filepath = MakePath(directory, filename);
        myMaterial.specularTextureIdx = LoadTexture2D(app, filepath.str);
    }
    if (material->GetTextureCount(aiTextureType_NORMALS) > 0)
    {
        material->GetTexture(aiTextureType_NORMALS, 0, &aiFilename);
        String filename = MakeString(aiFilename.C_Str());
        String filepath = MakePath(directory, filename);
        myMaterial.normalsTextureIdx = LoadTexture2D(app, filepath.str);
    }
    if (material->GetTextureCount(aiTextureType_HEIGHT) > 0)
    {
        material->GetTexture(aiTextureType_HEIGHT, 0, &aiFilename);
        String filename = MakeString(aiFilename.C_Str());
        String filepath = MakePath(directory, filename);
        myMaterial.bumpTextureIdx = LoadTexture2D(app, filepath.str);
    }

    //myMaterial.createNormalFromBump();
}

void ProcessAssimpNode(const aiScene* scene, aiNode* node, Mesh* myMesh, u32 baseMeshMaterialIndex, std::vector<u32>& submeshMaterialIndices)
{
    // process all the node's meshes (if any)
    for (unsigned int i = 0; i < node->mNumMeshes; i++)
    {
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        ProcessAssimpMesh(scene, mesh, myMesh, baseMeshMaterialIndex, submeshMaterialIndices);
    }

    // then do the same for each of its children
    for (unsigned int i = 0; i < node->mNumChildren; i++)
    {
        ProcessAssimpNode(scene, node->mChildren[i], myMesh, baseMeshMaterialIndex, submeshMaterialIndices);
    }
}

void CreateSphere(App* app) 
{
    const int H = 32;
    const int V = 16;
    struct Vertex { vec3 pos; vec3 norm; };

    Vertex sphere[H][V + 1];
    for (int h = 0; h < H; ++h)
    {
        for (int v = 0; v < V; ++v)
        {
            float nh = float(h) / H;
            float nv = float(v) / V - 0.5f;
            float angleh = 2 * PI * nh;
            float anglev = -PI * nv;
            sphere[h][v].pos.x = sinf(angleh) * cosf(anglev);
            sphere[h][v].pos.y = -sinf(anglev);
            sphere[h][v].pos.z = cosf(angleh) * cosf(anglev);
            sphere[h][v].norm = sphere[h][v].pos;
        }
    }

    unsigned int sphereIndices[H][V][6];

    for (unsigned int h = 0; h < H; ++h)
    {
        for (unsigned int v = 0; v < V; ++v)
        {
            sphereIndices[H][V][0] = (h+0) * (V+1) + v;
            sphereIndices[H][V][1] = ((h+1)%H) * (V+1) + v;
            sphereIndices[H][V][2] = ((h+1)%H) * (V+1) + v + 1;
            sphereIndices[H][V][3] = (h+0) * (V+1) + v;
            sphereIndices[H][V][4] = ((h+1)%H) * (V+1) + v + 1;
            sphereIndices[H][V][5] = (h + 0) * (V + 1) + v + 1;
        }
    }
    
    
    
    Material SphereMaterial;
    
    SphereMaterial.albedo = vec3(1.f, 1.f, 1.f);
    SphereMaterial.albedoTextureIdx = (u32)1;

    app->materials.push_back(SphereMaterial);

    Mesh SphereMesh;
    

    Model SphereModel;

    SphereModel.materialIdx;
    SphereModel.meshIdx;

    app->models.push_back(SphereModel);
}

u32 LoadModel(App* app, const char* filename)
{
    const aiScene* scene = aiImportFile(filename,
        aiProcess_Triangulate |
        aiProcess_GenSmoothNormals |
        aiProcess_CalcTangentSpace |
        aiProcess_JoinIdenticalVertices |
        aiProcess_PreTransformVertices |
        aiProcess_ImproveCacheLocality |
        aiProcess_OptimizeMeshes |
        aiProcess_SortByPType);

    if (!scene)
    {
        ELOG("Error loading mesh %s: %s", filename, aiGetErrorString());
        return UINT32_MAX;
    }

    app->meshes.push_back(Mesh{});
    Mesh& mesh = app->meshes.back();
    u32 meshIdx = (u32)app->meshes.size() - 1u;

    app->models.push_back(Model{});
    Model& model = app->models.back();
    model.meshIdx = meshIdx;
    u32 modelIdx = (u32)app->models.size() - 1u;

    String directory = GetDirectoryPart(MakeString(filename));

    // Create a list of materials
    u32 baseMeshMaterialIndex = (u32)app->materials.size();
    for (unsigned int i = 0; i < scene->mNumMaterials; ++i)
    {
        app->materials.push_back(Material{});
        Material& material = app->materials.back();
        ProcessAssimpMaterial(app, scene->mMaterials[i], material, directory);
    }

    ProcessAssimpNode(scene, scene->mRootNode, &mesh, baseMeshMaterialIndex, model.materialIdx);

    aiReleaseImport(scene);

    u32 vertexBufferSize = 0;
    u32 indexBufferSize = 0;

    for (u32 i = 0; i < mesh.submeshes.size(); ++i)
    {
        vertexBufferSize += mesh.submeshes[i].vertices.size() * sizeof(float);
        indexBufferSize += mesh.submeshes[i].indices.size() * sizeof(u32);
    }

    glGenBuffers(1, &mesh.vertexBufferHandle);
    glBindBuffer(GL_ARRAY_BUFFER, mesh.vertexBufferHandle);
    glBufferData(GL_ARRAY_BUFFER, vertexBufferSize, NULL, GL_STATIC_DRAW);

    glGenBuffers(1, &mesh.indexBufferHandle);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.indexBufferHandle);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexBufferSize, NULL, GL_STATIC_DRAW);

    glGenBuffers(1, &mesh.uniformBufferHandle);
    glBindBuffer(GL_UNIFORM_BUFFER, mesh.uniformBufferHandle);
    glBufferData(GL_UNIFORM_BUFFER, app->maxUniformBufferSize, NULL, GL_STREAM_DRAW);

    u32 indicesOffset = 0;
    u32 verticesOffset = 0;

    for (u32 i = 0; i < mesh.submeshes.size(); ++i)
    {
        const void* verticesData = mesh.submeshes[i].vertices.data();
        const u32   verticesSize = mesh.submeshes[i].vertices.size() * sizeof(float);
        glBufferSubData(GL_ARRAY_BUFFER, verticesOffset, verticesSize, verticesData);
        mesh.submeshes[i].vertexOffset = verticesOffset;
        verticesOffset += verticesSize;

        const void* indicesData = mesh.submeshes[i].indices.data();
        const u32   indicesSize = mesh.submeshes[i].indices.size() * sizeof(u32);
        glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, indicesOffset, indicesSize, indicesData);
        mesh.submeshes[i].indexOffset = indicesOffset;
        indicesOffset += indicesSize;
    }

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    return modelIdx;
}




void Init(App* app)
{
    // TODO: Initialize your resources here!
    // - vertex buffers
    // - element/index buffers
    // - vaos
    // - programs (and retrieve uniform indices)
    // - textures

    //app->mode = Mode_TexturedQuad;
    app->mode = Mode_AlbedoPatrick;

    // Camera
    
    app->camera.SetValues();

    glGetIntegerv(GL_MAX_UNIFORM_BLOCK_SIZE, &app->maxUniformBufferSize);
    glGetIntegerv(GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT, &app->uniformBlockAlignment);

    // Geometry
    glGenBuffers(1, &app->embeddedVertices);
    glBindBuffer(GL_ARRAY_BUFFER, app->embeddedVertices);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glGenBuffers(1, &app->embeddedElements);
    glBindBuffer(GL_ARRAY_BUFFER, app->embeddedElements);
    glBufferData(GL_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // Attribute State
    glGenVertexArrays(1, &app->vao);
    glBindVertexArray(app->vao);
    glBindBuffer(GL_ARRAY_BUFFER, app->embeddedVertices);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(VertexV3V2), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(VertexV3V2), (void*)12);
    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, app->embeddedElements);
    glBindVertexArray(0);

    // Program Initialization
    app->texturedMeshProgramIdx = LoadProgram(app, "AlbedoPatrick.glsl", "SHOW_TEXTURED_MESH");
    Program& texturedMeshProgram = app->programs[app->texturedMeshProgramIdx];
    LoadProgramAttributes(texturedMeshProgram);
    
    // Texture initialization
    //app->diceTexIdx = LoadTexture2D(app, "dice.png");
    app->whiteTexIdx = LoadTexture2D(app, "color_white.png");
    app->blackTexIdx = LoadTexture2D(app, "color_black.png");
    app->normalTexIdx = LoadTexture2D(app, "color_normal.png");
    app->magentaTexIdx = LoadTexture2D(app, "color_magenta.png");
    
    //LoadModel(app, "Models/Sphere/sphere.obj");
    
    //Objects
    Object object;
    object.position = vec3(0.0f, 0.0f, 0.0f);
    object.worldMatrix = identityMatrix;
    object.modelIndex = LoadModel(app, "Models/Patrick/Patrick.obj");
    app->objects.push_back(object);

    glEnable(GL_DEPTH_TEST);
    glDebugMessageCallback(OnGlError, app);
}

void Gui(App* app)
{
    //ImGui::Begin("Info");
    //ImGui::Text("FPS: %f", 1.0f/app->deltaTime);
    //ImGui::End();
}

void Update(App* app)
{
    // You can handle app->input keyboard/mouse here
    app->camera.UpdateCamera(app);

    for (u64 i = 0; i < app->programs.size(); i++)
    {
        Program& program = app->programs[i];
        u64 currentTimestamp = GetFileLastWriteTimestamp(program.filepath.c_str());
        if (currentTimestamp > program.lastWriteTimestamp)
        {
            glDeleteProgram(program.handle);
            String programSource = ReadTextFile(program.filepath.c_str());
            const char* programName = program.programName.c_str();
            program.handle = CreateProgramFromSource(programSource, programName);
            program.lastWriteTimestamp = currentTimestamp;
        }
    }

    for (int i = 0; i < app->objects.size(); i++)
    {
        Object& Object = app->objects[i];

        //update transform
        float aspectRatio = (float)app->displaySize.x / (float)app->displaySize.y;
        mat4 projectionMatrix = glm::perspective(glm::radians(app->camera.fov), aspectRatio, app->camera.zNear, app->camera.zFar);
        mat4 view = glm::lookAt(app->camera.Position, app->camera.currentReference, vec3(0, 1, 0));

        //update transform and view matrices
        //sceneObject.worldMatrix = IdentityMatrix; --> only at the start;
        Object.worldViewProjection = projectionMatrix * view * Object.worldMatrix;

        //opengl stuff
        glBindBuffer(GL_UNIFORM_BUFFER, app->meshes[app->models[Object.modelIndex].meshIdx].uniformBufferHandle);
        u8* bufferData = (u8*)glMapBuffer(GL_UNIFORM_BUFFER, GL_WRITE_ONLY);
        u32 bufferHead = 0;

        memcpy(bufferData + bufferHead, glm::value_ptr(Object.worldMatrix), sizeof(mat4));
        bufferHead += sizeof(mat4);

        memcpy(bufferData + bufferHead, glm::value_ptr(Object.worldViewProjection), sizeof(mat4));
        bufferHead += sizeof(mat4);

        glUnmapBuffer(GL_UNIFORM_BUFFER);
    }

}

void Render(App* app)
{
   
    switch (app->mode)
    {
    case Mode_TexturedQuad:
    {
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glViewport(0, 0, app->displaySize.x, app->displaySize.y);

        Program& programTexturedGeometry = app->programs[app->texturedMeshProgramIdx];
        glUseProgram(programTexturedGeometry.handle);
        glBindVertexArray(app->vao);

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        glUniform1i(app->programUniformTexture, 0);
        glActiveTexture(GL_TEXTURE0);
        GLuint textureHandle = app->textures[app->diceTexIdx].handle;
        glBindTexture(GL_TEXTURE_2D, textureHandle);

        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0);

        glBindVertexArray(0);
        glUseProgram(0);

    }
    break;
    case Mode_AlbedoPatrick:
    {

        // - clear the framebuffer
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // - set the viewport
        glViewport(0, 0, app->displaySize.x, app->displaySize.y);

        // - set the blending state
        glEnable(GL_BLEND);

        for (int j = 0; j < app->objects.size(); j++)
        {
            Program& texturedMeshProgram = app->programs[app->texturedMeshProgramIdx];
            glUseProgram(texturedMeshProgram.handle);

            Model& model = app->models[app->objects[j].modelIndex];
            Mesh& mesh = app->meshes[model.meshIdx];

            u32 blockOffset = 0;
            u32 blockSize = sizeof(mat4) * 2;

            for (u32 i = 0; i < mesh.submeshes.size(); ++i)
            {
                glBindBufferRange(GL_UNIFORM_BUFFER, BINDING(1), mesh.uniformBufferHandle, blockOffset, blockSize);

                GLuint vao = FindVAO(mesh, i, texturedMeshProgram);
                glBindVertexArray(vao);

                u32 submeshMaterialIdx = model.materialIdx[i];
                Material& submeshMaterial = app->materials[submeshMaterialIdx];

                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, app->textures[submeshMaterial.albedoTextureIdx].handle);
                glUniform1i(app->programUniformTexture, 0);//

                Submesh& submesh = mesh.submeshes[i];
                glDrawElements(GL_TRIANGLES, submesh.indices.size(), GL_UNSIGNED_INT, (void*)(u64)submesh.indexOffset);
            }
        }        
    }
    break;

    default:;
    }

    glDebugMessageCallback(OnGlError, app);
}

void Camera::SetValues()
{

    X = vec3(1.0f, 0.0f, 0.0f);
    Y = vec3(0.0f, 1.0f, 0.0f);
    Z = vec3(0.0f, 0.0f, 1.0f);

    sensitivity = 0.5f;
    Position = vec3(5, 5.0, 5.0f);
    currentReference = vec3(0.0f, 0.0f, 0.0f);
    speed = 0.01f;
    zNear = 0.1f;
    zFar = 1000.f;
    fov = 60.f;;

    CalculateViewMatrix();
}

void Camera::CalculateViewMatrix()
{
    {
        ViewMatrix = mat4(X.x, Y.x, Z.x, 0.0f, X.y, Y.y, Z.y, 0.0f, X.z, Y.z, Z.z, 0.0f, -dot(X, Position), -dot(Y, Position), -dot(Z, Position), 1.0f);
        ViewMatrixInverse = inverse(ViewMatrix);
    }
}

void Camera::UpdateCamera(App* app)
{

    vec3 newPos(0, 0, 0);
    float spd = speed * app->deltaTime * 1000.f;

    // keyboard movement

    if (app->input.keys[Key::K_Q])
    {
        newPos.y -= spd;
    }

    if (app->input.keys[Key::K_E])
    {
        newPos.y += spd;
    }

    if (app->input.keys[Key::K_W])
    {
        newPos -= Z * spd;
    }

    if (app->input.keys[Key::K_S])
    {
        newPos += Z * spd;
    }

    if (app->input.keys[Key::K_A])
    {
        newPos -= X * spd;
    }

    if (app->input.keys[Key::K_D])
    {
        newPos += X * spd;
    }

    Position += newPos;
    currentReference += newPos;

    //mouse movement

    if (!ImGui::IsWindowHovered(ImGuiHoveredFlags_AnyWindow) && app->input.mouseButtons[MouseButton::RIGHT])
    {
        float dx = app->input.mouseDelta.x;
        float dy = app->input.mouseDelta.y;

        Position -= currentReference;

        if (dx != 0)
        {
            float DeltaX = (float)dx * sensitivity;

            X = rotate(X, DeltaX, vec3(0.0f, 1.0f, 0.0f));
            Y = rotate(Y, DeltaX, vec3(0.0f, 1.0f, 0.0f));
            Z = rotate(Z, DeltaX, vec3(0.0f, 1.0f, 0.0f));
        }

        if (dy != 0)
        {
            float DeltaY = (float)dy * sensitivity;

            Y = rotate(Y, DeltaY, X);
            Z = rotate(Z, DeltaY, X);

            if (Y.y < 0.0f)
            {
                Z = vec3(0.0f, Z.y > 0.0f ? 1.0f : -1.0f, 0.0f);
                Y = cross(Z, X);
            }

        }
        Position = currentReference + Z * length(Position);

    }


    if (!ImGui::IsWindowHovered(ImGuiHoveredFlags_AnyWindow) && app->input.mouseButtons[MouseButton::LEFT])
    {
        float dx = app->input.mouseDelta.x;
        float dy = app->input.mouseDelta.y;

        if (dx != 0)
        {
            float DeltaX = (float)dx * sensitivity * 0.5f;

            X = rotate(X, DeltaX, vec3(0.0f, 1.0f, 0.0f));
            Y = rotate(Y, DeltaX, vec3(0.0f, 1.0f, 0.0f));
            Z = rotate(Z, DeltaX, vec3(0.0f, 1.0f, 0.0f));
        }

        if (dy != 0)
        {
            float DeltaY = (float)dy * sensitivity * 0.5f;

            Y = rotate(Y, DeltaY, X);
            Z = rotate(Z, DeltaY, X);

            if (Y.y < 0.0f)
            {
                Z = vec3(0.0f, Z.y > 0.0f ? 1.0f : -1.0f, 0.0f);
                Y = cross(Z, X);
            }
        }
        currentReference = Position - Z * length(Position);
    }

    CalculateViewMatrix();



}
