#version 460 core

out vec3 fragPos;
out vec2 uv;

uniform mat4 VP;

uniform vec3 camPos;
uniform vec3 viewDir;

struct Branch {
	vec3 start;
	vec3 end;
    vec3 mid;
	float lowRadius;
	float highRadius;
	vec3 camProjected;
	vec3 camCross;
    vec3 color;
    float branchLength;
    float startLength;
    float uvOffset;
};

uniform Branch branch;

void main()
{
	float back = float(gl_VertexID > 3);

	float bottom = float(gl_VertexID < 2 || gl_VertexID > 5);

	float up = float(gl_VertexID > 1 && gl_VertexID < 6);

	float right = gl_VertexID % 2;

	float rightSide = 2.0 * (right - 0.5);
	
	float front = 2.0 * (0.5 - back);

	float maxRadius = max(branch.lowRadius, branch.highRadius);
		float mid = 0.0;

	vec3 pos = front * maxRadius * branch.camProjected + bottom * branch.start + (1.0 - bottom) * branch.end + maxRadius * rightSide * branch.camCross;

	vec3 center = (branch.end + branch.start) / 2.0;

	vec3 dif = pos-center;
	pos = dif * (2. + branch.lowRadius) + center;

	fragPos = pos;

	gl_Position = VP * vec4(pos, 1.0);
	uv = vec2(right, front * 0.5 + bottom * 0.5);
}