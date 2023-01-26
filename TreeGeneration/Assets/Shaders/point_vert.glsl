#version 460 core

uniform mat4 VP;
uniform vec3 pos;
void main()
{
	gl_Position = VP * vec4(pos, 1.0f);
}