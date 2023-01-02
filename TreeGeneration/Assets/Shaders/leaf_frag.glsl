#version 460 core
out vec4 FragColor;

layout(early_fragment_tests) in;
in vec3 fragPos;
in vec3 normal;
in vec2 uv;

uniform sampler2D leafTex;

uniform vec3 camPos;

uniform vec3 ambientColor;
uniform vec3 lightDir;
uniform vec3 lightColor;

float diffuse(vec3 norm, vec3 lightDir) {
    return clamp(dot(norm, -lightDir), 0.2, 1.0);
}

float specular(vec3 norm, vec3 viewDir, vec3 lightDir, float specularStrength, int shininess) {
    vec3 halfwayDir = normalize(-lightDir + viewDir);

    float spec = pow(max(dot(halfwayDir, norm), 0.0), shininess);
    return spec * specularStrength;
}

vec3 calcLight(vec3 viewDir, vec3 norm, vec3 diffCol) {
    float diff = diffuse(norm, lightDir);
    float spec = specular(norm, viewDir, lightDir, 0.5, 32);
    
    vec3 light = (spec + diff) * lightColor + ambientColor;

    vec3 reflectDir = reflect(normalize(-lightDir), normalize(norm));

    return light * diffCol;
}

void main()
{    
    vec4 col = texture(leafTex, uv);
    vec4 al = textureLod(leafTex, uv, 0);
    if(al.a < 0.5)
        discard;
    
    vec3 norm = normalize(normal);
    vec3 viewDir = normalize(camPos - fragPos);
    norm *= 2.0 * (float(gl_FrontFacing) - 0.5);

    vec4 color = vec4(calcLight(viewDir, norm, col.xyz), 1.0);
    FragColor = color;
}