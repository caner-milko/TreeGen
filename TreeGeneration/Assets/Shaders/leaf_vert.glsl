#version 460 core
layout (location = 0) in vec2 aPos;

out vec3 fragPos;
out vec3 normal;
out vec2 uv;

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

layout(std430, binding=0) buffer leaf_data {
    mat4 models[];
};

void main()
{
    mat4 model = models[gl_InstanceID];

    fragPos = (model * vec4(aPos, 0.0, 1.0)).xyz;
    gl_Position = cam.vp * model * vec4(aPos, 0.0, 1.0);
    uv = vec2(aPos.x, aPos.y) + vec2(0.5, 0.0);
    normal = mat3(model) * vec3(0.0, 0.0, 1.0);
}  