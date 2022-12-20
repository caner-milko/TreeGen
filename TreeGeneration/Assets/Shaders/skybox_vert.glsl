#version 460 core
layout (location = 0) in vec3 aPos;

out vec3 TexCoords;

uniform mat4 skybox_vp;

void main()
{
    TexCoords = aPos;
    gl_Position = (skybox_vp * vec4(aPos, 1.0)).xyww;
}  