#version 460 core
out vec4 FragColor;

in vec3 fragPos;
in vec3 normal;
in vec2 uv;

struct Camera {
    mat4 vp;
    vec4 pos_near;
    vec4 dir_far;
    vec4 ortho;
    vec2 aspectRatio_projection;
};

layout(std140, binding=0) uniform Cam {
    Camera cam;
};

layout(std140, binding=1) uniform Light {
    vec4 lightColor; 
    vec4 ambientColor;
    Camera lightCam;
};

uniform sampler2D leafTex;

float diffuse(vec3 norm, vec3 lightDir) {
    return clamp(dot(norm, -lightDir), 0.5, 1.0);
}

float specular(vec3 norm, vec3 viewDir, vec3 lightDir, float specularStrength, int shininess) {
    vec3 halfwayDir = normalize(-lightDir + viewDir);

    float spec = pow(max(dot(halfwayDir, norm), 0.0), shininess);
    return spec * specularStrength;
}

vec3 calcLight(vec3 viewDir, vec3 norm, vec3 diffCol) {
    float diff = diffuse(norm, lightCam.dir_far.xyz);
    float spec = specular(norm, viewDir, lightCam.dir_far.xyz, 0.7, 32);
    
    vec3 light = (spec + diff) * lightColor.xyz + ambientColor.xyz;

    return light * diffCol;
}

void main()
{    
    vec4 col = texture(leafTex, uv);
    vec4 al = textureLod(leafTex, uv, 0);
    if(al.a < 0.5)
        discard;
    
    vec3 norm = normalize(normal);
    vec3 viewDir = normalize(cam.pos_near.xyz - fragPos);

    norm *= 2.0 * (float(gl_FrontFacing) - 0.5);

    float multiplier = 1.5;

    FragColor = vec4(clamp(calcLight(viewDir, norm, col.xyz) * multiplier, 0.0, 1.0), al.a);
}