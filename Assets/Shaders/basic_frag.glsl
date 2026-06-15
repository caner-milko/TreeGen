#version 460 core
out vec4 FragColor;

in vec3 fragPos;

uniform vec3 color;

uniform vec3 camPos;

uniform vec3 lightDir;
uniform vec3 lightColor;
uniform vec3 ambientColor;

float diffuse(vec3 normal, vec3 lightDir) {
    return max(dot(normal, -lightDir), 0.0);
}

float specular(vec3 normal, vec3 viewDir, vec3 lightDir, float specularStrength, int shininess) {
    vec3 halfwayDir = normalize(-lightDir + viewDir);

    float spec = pow(max(dot(halfwayDir, normal), 0.0), shininess);
    return spec * specularStrength;
}

vec3 calcLight(vec3 viewDir, vec3 normal) {
    float diff = diffuse(normal, lightDir);

    float spec = specular(normal, viewDir, lightDir, 0.5, 32);
    
    vec3 light = (spec + diff) * lightColor + ambientColor;

    vec3 color = light * color;

    return color;

}

void main()
{
    FragColor = vec4(calcLight(normalize(camPos - fragPos), vec3(0.0, 1.0, 0.0)), 1.0);
} 