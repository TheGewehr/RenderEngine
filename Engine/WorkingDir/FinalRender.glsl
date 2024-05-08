///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
#ifdef SHOW_TEXTURED_MESH

#define MaxLightNumber 3

#if defined(VERTEX) ///////////////////////////////////////////////////

struct Light {
    unsigned int type;   // 0 for directional, 1 for point
    vec3 color;
    vec3 direction;      // For directional lights
    vec3 position;       // For point lights
};

layout(location = 0) in vec3 aPosition; // world space
layout(location = 1) in vec3 aNormal; // world space
layout(location = 2) in vec2 aTexCoord;

layout(binding = 0, std140) uniform globalParams {
    vec3 uCameraPosition;
    unsigned int uLightCount;
    Light uLight[MaxLightNumber];
};

layout(binding = 1, std140) uniform localParams {
    mat4 uWorldMatrix;
    mat4 uWorldViewProjectionMatrix;
};

out vec3 vPosition;
out vec3 vNormal;
out vec3 vViewDir;
out vec3 vLightDir[MaxLightNumber];
out vec3 vLightColor[MaxLightNumber];
out vec2 vTexCoord;

void main() {

    vPosition = vec3(uWorldMatrix * vec4(aPosition, 1.0));
    vNormal = normalize(mat3(uWorldMatrix) * aNormal);
    vViewDir = normalize(uCameraPosition - vPosition);
    vTexCoord = aTexCoord;

	//for(int i = 0; i < MaxLightNumber; i++) {
    //// Use a test directional light pointing in the Z direction
    //vLightDir[0] = vec3(0.0,0.0,-1.0); // Adjust this as necessary to point towards your geometry
    //vLightColor[0] = vec3(1.0, 1.0, 1.0); // Bright white light
	//}

    //vLightDir[1] = normalize(vec3(1.0f,1.0f,1.0f) - vPosition); // Adjust this as necessary to point towards your geometry
    //vLightColor[1] = vec3(1.0, 0.0, 1.0); // Bright white light

    //vLightDir[2] = normalize(vec3(1.0f,-1.0f,2.0f) - vPosition); // Adjust this as necessary to point towards your geometry
    //vLightColor[2] = vec3(1.0, 1.0, 0.0); // Bright white light

    //Light lighty;
    //lighty.type = 1;
    //lighty.color = vec3(1.0, 1.0, 1.0);
    //lighty.direction = vec3(1.0, 1.0, 1.0);
    //lighty.position = vec3(1.0, 1.0, 1.0);    

    for(int i = 0; i < uLightCount; i++) {
        if(uLight[i].type == 0) {  // Directional light
            vLightDir[i] = normalize(uLight[i].direction);
        } else if(uLight[i].type == 1) {  // Point light
            vLightDir[i] = normalize(vec3(1.0f,1.0f,1.0f) - vPosition);
        }
        vLightColor[i] = uLight[i].color;
    }

    gl_Position = uWorldViewProjectionMatrix * vec4(aPosition, 1.0);
}

#elif defined(FRAGMENT) ///////////////////////////////////////////////

in vec3 vPosition;
in vec3 vNormal;
in vec3 vViewDir;
in vec3 vLightDir[MaxLightNumber];
in vec3 vLightColor[MaxLightNumber];
in vec2 vTexCoord;

uniform sampler2D uTexture;
layout(location = 0) out vec4 oColor;

void main() {
    vec3 norm = normalize(vNormal);
    vec3 resultColor = vec3(0.0);
    vec4 texColor = texture(uTexture, vTexCoord);

    for (int i = 0; i < 1; i++) {
        vec3 lightDir = vLightDir[i];
        vec3 lightColor = vLightColor[i];

        // Diffuse component
        float diff = max(dot(norm, lightDir), 0.0);
        vec3 diffuse = diff * lightColor;

        // Specular component
        vec3 reflectDir = reflect(lightDir, norm);
        float spec = pow(max(dot(vViewDir, reflectDir), 0.0), 300);
        vec3 specular = spec * lightColor;

        resultColor += (diffuse + specular) * vec3(texColor);
    }

    oColor = vec4(resultColor, texColor.a);
}


#endif
#endif