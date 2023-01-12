#version 460 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;

out vec3 fragPos;
out vec3 normal;
out vec2 uv;

uniform mat4 VP;
uniform mat4 model;

void main()
{
    vec3 pos = aPos;
    pos.y /= 5.0;
    fragPos = pos; //(model * vec4(uv.x, 0.0, uv.y, 1.0)).xyz;
    gl_Position = VP * vec4(pos, 1.0);
    uv = pos.xz;
    normal = aNormal;
}  