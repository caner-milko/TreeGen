#version 460 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;

out vec3 fragPos;
out vec2 uv;
out mat3 TBN;

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

struct TerrainMaterial {
    sampler2D grassTex;
    sampler2D dirtTex;
    float grassColorMultiplier;
    float dirtColorMultiplier;
    sampler2D normalMap;
    float normalMapStrength;
    float uvScale;
};

uniform TerrainMaterial material;

uniform mat4 model;
uniform mat3 ITmodel;



void main()
{
    vec3 pos = aPos;
    pos.xz -= vec2(0.5);
    vec4 transformed = model * vec4(pos, 1.0);
    fragPos = transformed.xyz;
    gl_Position = cam.vp * transformed;
    uv = aPos.xz * material.uvScale;
    vec3 N = normalize(ITmodel * aNormal);
    vec3 T = vec3(0.0, 0.0, -1.0);
    T = normalize(T - dot(T, N) * N);
    vec3 B = cross(N, T);
    TBN = mat3(T, B, N);
}  