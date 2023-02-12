#version 460 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;

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

uniform mat4 model;
uniform mat4 ITmodel;



void main()
{
    vec3 pos = aPos;
    pos.xz -= vec2(0.5);
    vec4 transformed = model * vec4(pos, 1.0);
    fragPos = transformed.xyz;
    gl_Position = cam.vp * transformed;
    uv = aPos.xz * 3.0f;
    normal = normalize(mat3(transpose(inverse(model))) * aNormal);
    //normal = aNormal;
}  