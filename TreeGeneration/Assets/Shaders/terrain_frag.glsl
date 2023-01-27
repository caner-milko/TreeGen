#version 460 core
out vec4 FragColor;

in vec3 fragPos;
in vec3 normal;
in vec2 uv;

uniform sampler2D grassTex;

uniform vec3 camPos;

uniform vec3 lightDir;
uniform vec3 lightColor;
uniform vec3 ambientColor;

uniform sampler2D shadowMap;
uniform mat4 lightVP;

float diffuse(vec3 norm, vec3 lightDir) {
    return max(dot(norm, -lightDir), 0.0);
}

float specular(vec3 norm, vec3 viewDir, vec3 lightDir, float specularStrength, int shininess) {
    vec3 halfwayDir = normalize(-lightDir + viewDir);

    float spec = pow(max(dot(halfwayDir, norm), 0.0), shininess);
    return spec * specularStrength;
}

vec3 calcLight(vec3 viewDir, vec3 norm, vec3 col) {
    float diff = diffuse(norm, lightDir);

    float spec = specular(norm, viewDir, lightDir, 0.0, 32);
    vec3 light = clamp((spec + diff), 0.0, 1.0) * lightColor + ambientColor;

    vec3 color = light * col;
    
    return color;

}

float calcShadow(vec3 pos) {
    vec2 poissonDisk[4] = vec2[](
        vec2( -0.94201624, -0.39906216 ),
        vec2( 0.94558609, -0.76890725 ),
        vec2( -0.094184101, -0.92938870 ),
        vec2( 0.34495938, 0.29387760 )
    );
    vec4 posLightSpace = lightVP * vec4(pos, 1.0);

    vec3 projCoords = posLightSpace.xyz / posLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5; 

    float bias = 0.001;
    float visibility = 1.0;
    for (int i=0;i<4;i++){
            visibility -= 0.2 * float((projCoords.z - bias)  > texture(shadowMap, projCoords.xy + poissonDisk[i]/700.0).r);
    }

    return visibility;
}

void main()
{
    vec3 norm = normalize(normal);
    vec3 col = texture(grassTex, uv * 5.0).xyz;
    FragColor = vec4(calcLight(normalize(camPos - fragPos), norm, col) * calcShadow(fragPos), 1.0);
    //FragColor = vec4(norm, 1.0);
} 