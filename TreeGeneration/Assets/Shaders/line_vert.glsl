#version 460 core

uniform mat4 VP;

uniform vec3 pos1;
uniform vec3 pos2;

void main()
{

	if(gl_VertexID == 0) {
		gl_Position = VP * vec4(pos1, 1.0f);
	}
	else {
		gl_Position = VP * vec4(pos2, 1.0f);
	}

}