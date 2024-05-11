///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
#ifdef SHOW_GEO

#if defined(VERTEX) ///////////////////////////////////////////////////

layout(location = 0) in vec3 aPosition; // world space
layout(location = 1) in vec3 aNormal; // world space
layout(location = 2) in vec2 aTexCoord;

layout(binding = 1, std140) uniform localParams {
    mat4 uWorldMatrix;
    mat4 uViewMatrix;
    mat4 uProjectionMatrix;
};

out vec3 FragPos;
out vec3 Normal;
out vec2 TexCoords;

void main()
{
    FragPos = vec3(uWorldMatrix * vec4(aPos, 1.0));
    Normal = mat3(transpose(inverse(uWorldMatrix))) * aNormal;
    TexCoords = aTexCoords;
    gl_Position = uProjectionMatrix * uViewMatrix * vec4(FragPos, 1.0);
}

#elif defined(FRAGMENT) ///////////////////////////////////////////////

layout(location = 0) out vec3 gPosition;
layout(location = 1) out vec3 gNormal;
layout(location = 2) out vec2 gTexCoords;


in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;

void main()
{
    gPosition = FragPos;
    gNormal = normalize(Normal);
    gTexCoords = TexCoords;
}

#endif
#endif


// NOTE: You can write several shaders in the same file if you want as
// long as you embrace them within an #ifdef block (as you can see above).
// The third parameter of the LoadProgram function in engine.cpp allows
// chosing the shader you want to load by name.
