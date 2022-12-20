#version 460 core
layout (location = 0) in vec3 aPos;

out vec3 fragPos;
flat out int instanceID;

uniform mat4 VP;

struct BranchData {
    mat4 model;
	vec4 start;
    vec4 mid;
	vec4 end;
    vec4 color;
	float lowRadius;
	float highRadius;
    float startLength;
    float branchLength;
    vec4 uvOffset;
};

layout(std430, binding=0) buffer branch_data {
   BranchData branchs[];
};

void main()
{
    mat4 model = branchs[gl_InstanceID].model;
    fragPos = (model * vec4(aPos, 1.0)).xyz;
    gl_Position = VP * model * vec4(aPos, 1.0);
    instanceID = gl_InstanceID;
}  