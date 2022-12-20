#version 460 core
out vec4 FragColor;

in vec3 fragPos;

uniform vec3 color;

void main()
{
    FragColor = vec4(fragPos, 1.0);
} 