#version 330 core
layout (location = 0) in float empty;

uniform mat4 VP;
uniform vec3 pos;

void main()
{
	gl_Position = VP * vec4(pos, 1.0f);
}