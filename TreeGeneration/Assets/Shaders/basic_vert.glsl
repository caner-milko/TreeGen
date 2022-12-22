#version 460 core
layout (location = 0) in vec2 aPos;

out vec3 fragPos;

uniform mat4 VP;
uniform mat4 model;

void main()
{
    fragPos = (model * vec4(aPos, 0.0, 1.0)).xyz;
    gl_Position = VP * model * vec4(aPos, 0.0, 1.0);
}  