///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
#ifdef SHOW_GEO

#if defined(VERTEX) ///////////////////////////////////////////////////

layout(location = 0) in vec3 aPosition; // world space
layout(location = 1) in vec3 aNormal; 
layout(location = 2) in vec2 aTexCoord;

uniform mat4 uWorldMatrix;
uniform mat4 uViewMatrix;
uniform mat4 uProjectionMatrix;
//uniform mat3 u_NormalMatrix;

out vec3 FragPos;
out vec3 Normal;
out vec2 TexCoord;

void main()
{
    FragPos = vec3(uWorldMatrix * vec4(aPosition, 1.0));
    Normal = mat3(transpose(inverse(uWorldMatrix))) * aNormal;
    TexCoords = aTexCoords;
    gl_Position = uProjectionMatrix * uViewMatrix * vec4(FragPos, 1.0);
}

#elif defined(FRAGMENT) ///////////////////////////////////////////////

out vec4 oColor;

layout (location = 0) out vec3 gPosition;
layout (location = 1) out vec3 gNormal;
layout (location = 2) out vec4 gAlbedoSpec;

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoord;

uniform sampler2D textureOutput;
//uniform sampler2D texture_specular1;

void main()
{
    // store the fragment position vector in the first gbuffer texture
    gPosition = vec4(FragPos,1.0);
    // also store the per-fragment normals into the gbuffer
    gNormal = vec4(normalize(Normal),1.0);
    // and the diffuse per-fragment color
    gAlbedoSpec.rgb = texture(textureOutput, TexCoords).rgb;
    // store specular intensity in gAlbedoSpec's alpha component
    gAlbedoSpec.a = 0.5;
    
    oColor = gAlbedoSpec;
}

#endif
#endif


// NOTE: You can write several shaders in the same file if you want as
// long as you embrace them within an #ifdef block (as you can see above).
// The third parameter of the LoadProgram function in engine.cpp allows
// chosing the shader you want to load by name.
