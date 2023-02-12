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

struct ColoredLine {
	vec4 pos1;
	vec4 pos2;
	vec4 color;
};


layout(std430, binding=0) buffer line_data {
    ColoredLine lines[];
};


void main()
{
	ColoredLine line = lines[gl_InstanceID];
	vec4 pos;
	if(gl_VertexID == 0) {
		pos = cam.vp * vec4(line.pos1.xyz, 1.0);
	} else {
		pos = cam.vp * vec4(line.pos2.xyz, 1.0);
	}
	gl_Position = pos;
	color = line.color.xyz;
}