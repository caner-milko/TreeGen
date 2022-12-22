#version 460 core
out vec4 FragColor;

in vec3 fragPos;
in vec3 normal;
in vec2 uv;

uniform sampler2D leafTex;

uniform vec3 ambientColor;
uniform vec3 lightDir;
uniform vec3 lightColor;

void main()
{    
    vec4 col = texture(leafTex, uv);
    if(col.a < 0.5)
        discard;
    vec3 norm = normalize(normal);
    vec3 light = clamp(abs(dot(norm, -lightDir)), 0.2, 1.0) * lightColor + ambientColor;
    vec4 color = vec4(light, 1.0) * col;
    FragColor = pow(color, vec4(1.0/2.2));
}