#version 460 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 texCoords;
layout (location = 2) in vec3 aNormal;

out vec2 uv;

struct Camera {
    mat4 vp;
    vec4 pos_near;
    vec4 dir_far;
    vec4 ortho;
    vec2 aspectRatio_projection;
};

layout(std140, binding=1) uniform Light {
    vec4 lightColor; 
    vec4 ambientColor;
    Camera lightCam;
};

layout(std430, binding=0) buffer leaf_data {
    mat4 models[];
};

void main()
{
    mat4 model = models[gl_InstanceID];

    gl_Position = lightCam.vp * model * vec4(aPos, 1.0);
    uv = texCoords;
}  