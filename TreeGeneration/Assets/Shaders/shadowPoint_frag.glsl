#version 330 core
out vec4 FragColor;

uniform float shadow;

void main()
{
    FragColor = vec4(vec3(1.0 - sqrt(shadow/3.0)), 1.0);
} 