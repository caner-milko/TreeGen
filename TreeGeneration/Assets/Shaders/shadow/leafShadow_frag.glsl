#version 460 core

in vec2 uv;

uniform sampler2D leafTex;

void main()
{    
    vec4 al = textureLod(leafTex, uv, 0);
    if(al.a < 0.5)
        discard;
}