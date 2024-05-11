///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
#ifdef SHOW_LIGHTS

#define MaxLightNumber 8

#if defined(VERTEX) ///////////////////////////////////////////////////

layout(location = 0) in vec2 aPos;
layout(location = 1) in vec2 aTexCoords;

layout(binding = 0, std140) uniform globalParams {
    vec3 uCameraPosition;
    unsigned int uLightCount;
    Light uLight[MaxLightNumber];
};

out vec2 TexCoords;

void main()
{
    TexCoords = aTexCoords;
    gl_Position = vec4(aPos.x, aPos.y, 0.0, 1.0);
}   

#elif defined(FRAGMENT) ///////////////////////////////////////////////

out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gTexCoords;

struct Light {
    vec3 position;
    vec3 color;
    float intensity;
};

uniform Light lights[16];  // Example: array of lights
uniform int numLights;
uniform vec3 viewPos;

void main()
{
    vec3 albedo = texture(gTexCoords, TexCoords).rgb;
    vec3 pos = texture(gPosition, TexCoords).xyz;
    vec3 normal = normalize(texture(gNormal, TexCoords).xyz);
    vec3 ambient = 0.1 * albedo;

    vec3 lighting = ambient;
    for(int i = 0; i < numLights; i++)
    {
        // Simple directional light calculation
        vec3 lightDir = normalize(lights[i].position - pos);
        float diff = max(dot(normal, lightDir), 0.0);
        vec3 diffuse = diff * lights[i].color * albedo * lights[i].intensity;
        lighting += diffuse;
    }

    FragColor = vec4(lighting, 1.0);
}

#endif
#endif


// NOTE: You can write several shaders in the same file if you want as
// long as you embrace them within an #ifdef block (as you can see above).
// The third parameter of the LoadProgram function in engine.cpp allows
// chosing the shader you want to load by name.
