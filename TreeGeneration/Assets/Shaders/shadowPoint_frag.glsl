#version 460 core
out vec4 FragColor;

in float shadow;

void main()
{
    FragColor = vec4(vec3(1.0 - sqrt(shadow * 2.0/3.0)), 1.0);
   
} 