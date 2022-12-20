#version 460 core
out vec4 FragColor;

in vec3 fragPos;
in vec2 uv;

uniform sampler2D leafTex;

void main()
{    
    vec4 col = texture(leafTex, uv);
    if(col.a < 0.5)
        discard;
    FragColor = col;
}