#version 330 core
//layout (location = 0) in float id;

out vec3 fragPos;
out vec2 uv;

uniform mat4 VP;

uniform vec3 camPos;
uniform vec3 viewDir;

struct Branch {
	vec3 start;
	vec3 end;
	float lowRadius;
	float highRadius;
	vec3 camProjected;
	vec3 camCross;
};

uniform Branch branchMain;

void main()
{
	float back = float(gl_VertexID > 3);

	float bottom = float(gl_VertexID < 2 || gl_VertexID > 5);

	float up = float(gl_VertexID > 1 && gl_VertexID < 6);

	float right = gl_VertexID % 2;

	float rightSide = 2.0 * (right - 0.5);
	
	float front = 2.0 * (0.5 - back);

	float maxRadius = max(branchMain.lowRadius, branchMain.highRadius);
		float mid = 0.0;

	vec3 pos = front * maxRadius * branchMain.camProjected + bottom * branchMain.start + (1.0 - bottom) * branchMain.end + maxRadius * rightSide * branchMain.camCross;

	vec3 center = (branchMain.end + branchMain.start) / 2.0;

	vec3 dif = pos-center;
	pos = dif * (1. + branchMain.lowRadius *2.0) + center;

	fragPos = pos;

	gl_Position = VP * vec4(pos, 1.0);
	uv = vec2(right, front * 0.5 + bottom * 0.5);
}