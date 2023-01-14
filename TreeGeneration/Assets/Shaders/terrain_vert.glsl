#version 460 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;

out vec3 fragPos;
out vec3 normal;
out vec2 uv;

uniform mat4 VP;
uniform mat4 model;
uniform mat4 ITmodel;

void main()
{
    vec3 pos = aPos;
    pos.xz -= vec2(0.5);
    vec4 transformed = model * vec4(pos, 1.0);
    fragPos = transformed.xyz;
    gl_Position = VP * transformed;
    uv = aPos.xz;
    normal = normalize(mat3(transpose(inverse(model))) * aNormal);
    //normal = aNormal;
}  