#version 460 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;

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

uniform mat4 model;

void main()
{
    vec3 pos = aPos;
    pos.xz -= vec2(0.5);
    vec4 transformed = model * vec4(pos, 1.0);
    gl_Position = lightCam.vp * transformed;
    //normal = aNormal;
}  