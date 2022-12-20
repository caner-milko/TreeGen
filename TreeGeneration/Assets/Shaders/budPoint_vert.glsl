#version 460 core

out vec3 color;

uniform mat4 VP;

struct Bud {
	vec4 pos;
	vec4 color;
};

layout(std430, binding=0) buffer leaf_data {
    Bud buds[];
};


void main()
{
	Bud bud = buds[gl_InstanceID];
	gl_Position = VP * vec4(bud.pos.xyz, 1.0f);
	gl_PointSize = max(bud.color.w, 1.0) * 3.0;
	color = bud.color.xyz;
}