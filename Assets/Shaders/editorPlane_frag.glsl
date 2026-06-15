#version 460 core
out vec4 FragColor;

in vec2 uv;

uniform sampler2D map;

void main()
{
    FragColor = vec4(texture(map, uv).xyz, 1.0);
} 