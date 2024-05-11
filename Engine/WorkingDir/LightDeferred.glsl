///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
#ifdef SHOW_LIGHTS

#define MaxLightNumber 8

#if defined(VERTEX) ///////////////////////////////////////////////////

struct Light {
    unsigned int type;   // 0 for directional, 1 for point
    vec3 color;
    vec3 direction;      // For directional lights
    vec3 position;       // For point lights
};

layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec3 aNormal; // world space
layout(location = 2) in vec2 aTexCoord;

layout(binding = 0, std140) uniform globalParams {
    vec3 uCameraPosition;
    unsigned int uLightCount;
    Light uLight[MaxLightNumber];
};

out vec3 vPosition;
out vec3 vNormal;
out vec3 vLightDir[MaxLightNumber];
out vec3 vLightColor[MaxLightNumber];
out vec2 TexCoord;
flat out unsigned int vLightCount;

void main()
{
    TexCoord = aTexCoord;
    vPosition = vec3(uWorldMatrix * vec4(aPosition, 1.0));
    vNormal = normalize(mat3(uWorldMatrix) * aNormal);
    vLightCount = uLightCount;

    for(int i = 0; i < uLightCount; i++) {
        if(uLight[i].type == 0) {  // Directional light
            vLightDir[i] = normalize(uLight[i].direction);
        } else if(uLight[i].type == 1) {  // Point light
            vLightDir[i] = normalize(vec3(1.0f,1.0f,1.0f) - vPosition);
        }
        vLightColor[i] = uLight[i].color;
    }

    gl_Position = vec4(aPosition.x, aPosition.y, 0.0, 1.0);
}   

#elif defined(FRAGMENT) ///////////////////////////////////////////////

out vec4 oColor;

in vec3 vPosition;
in vec3 vNormal;
in vec3 vLightDir[MaxLightNumber];
in vec3 vLightColor[MaxLightNumber];
in vec2 TexCoord;
flat in unsigned int vLightCount;

uniform Light lights[16];  // Example: array of lights
uniform int numLights;
uniform vec3 viewPos;

void main()
{
    vec3 albedo = texture(TexCoord, TexCoord).rgb;
    vec3 pos = texture(vPosition, TexCoord).xyz;
    vec3 normal = normalize(texture(vNormal, TexCoord).xyz);
    vec3 ambient = 0.1 * albedo;

    vec3 lighting = ambient;
    vec4 texColor = texture(uTexture, vTexCoord);

    for (int i = 0; i < vLightCount; i++) 
    {
        // Simple directional light calculation
        vec3 lightDir = vLightDir[i];
        vec3 lightColor = vLightColor[i];

        float diff = max(dot(normal, lightDir), 0.0);
        vec3 diffuse = diff * lightColor * albedo * 1.0;

        vec3 reflectDir = reflect(-lightDir, normal);
        float spec = pow(max(dot(vViewDir, reflectDir), 0.0), 300);
        vec3 specular = spec * lightColor;

        lighting += (diffuse + specular) * vec3(texColor);
    }

    oColor = vec4(lighting, 1.0);
}

#endif
#endif


// NOTE: You can write several shaders in the same file if you want as
// long as you embrace them within an #ifdef block (as you can see above).
// The third parameter of the LoadProgram function in engine.cpp allows
// chosing the shader you want to load by name.
