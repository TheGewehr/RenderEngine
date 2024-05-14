///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
#ifdef SHOW_LIGHTS

#if defined(VERTEX) ///////////////////////////////////////////////////

layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec2 aTexCoord;

out vec2 TexCoord;

void main()
{
    TexCoord = aTexCoord; 

    gl_Position = vec4(aPosition, 1.0);
}   

#elif defined(FRAGMENT) ///////////////////////////////////////////////

in vec2 TexCoords;

out vec4 FragColor;

// G-buffer textures
uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gAlbedoSpec;

// Light properties
struct Light {
    vec3 Position;
    vec3 Color;

    // Attenuation
    float Linear;
    float Quadratic;
};
uniform Light light;

// Camera position
uniform vec3 viewPos;

// Number of samples (optional)
const int SAMPLE_COUNT = 16;

void main()
{
    // Retrieve data from G-buffer
    vec3 FragPos = texture(gPosition, TexCoords).rgb;
    vec3 Normal = texture(gNormal, TexCoords).rgb;
    vec3 Albedo = texture(gAlbedoSpec, TexCoords).rgb;
    float Specular = texture(gAlbedoSpec, TexCoords).a;

    // Calculate lighting
    vec3 ambient = 0.1 * Albedo;
    vec3 lightDir = normalize(light.Position - FragPos);
    float diff = max(dot(Normal, lightDir), 0.0);
    vec3 diffuse = diff * light.Color * Albedo;

    // Specular shading
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, Normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32.0);
    vec3 specular = light.Color * spec * Specular;

    // Attenuation
    float distance = length(light.Position - FragPos);
    float attenuation = 1.0 / (1.0 + light.Linear * distance + light.Quadratic * (distance * distance));

    // Combine results
    vec3 lighting = (ambient + diffuse + specular) * attenuation;
    FragColor = vec4(lighting, 1.0);
}

#endif
#endif


// NOTE: You can write several shaders in the same file if you want as
// long as you embrace them within an #ifdef block (as you can see above).
// The third parameter of the LoadProgram function in engine.cpp allows
// chosing the shader you want to load by name.
