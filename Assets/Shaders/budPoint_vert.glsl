#version 460 core

out vec3 color;

struct Camera {
    mat4 vp;
    vec4 pos_near;
    vec4 dir_far;
    vec4 ortho;
    vec2 aspectRatio_projection;
};

layout(std140, binding=0) uniform Cam {
    Camera cam;
};

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
	gl_Position = cam.vp * vec4(bud.pos.xyz, 1.0f);
	gl_PointSize = max(bud.color.w, 1.0) * 3.0;
	color = bud.color.xyz;
}