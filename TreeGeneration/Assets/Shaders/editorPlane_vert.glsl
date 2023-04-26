#version 460 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 texCoords;
layout (location = 2) in vec3 aNormal;

out vec2 uv;

uniform mat4 MVP;

void main()
{
    uv = texCoords;
    gl_Position = MVP * vec4(aPos, 1.0);
}  
