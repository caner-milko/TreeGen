#version 460 core

out float shadow;

uniform mat4 VP;

layout(std430, binding=0) buffer leaf_data {
    vec4 data[];
};


void main()
{
	gl_Position = VP * vec4(data[gl_InstanceID].xyz, 1.0f);
	shadow = data[gl_InstanceID].w;
	gl_PointSize = shadow * 2.0;
}