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

void main()
{
    vec3 norm = normalize(normal);
    vec3 col = texture(grassTex, uv * 5.0).xyz;
    FragColor = vec4(calcLight(normalize(camPos - fragPos), norm, vec3(1.0)), 1.0);
    //FragColor = vec4(norm, 1.0);
} 