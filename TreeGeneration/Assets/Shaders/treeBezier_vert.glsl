#version 460 core
layout (location = 0) in vec3 aPos;

out vec3 fragPos;
flat out int instanceID;
flat out int inside;

uniform mat4 VP;

uniform vec3 camPos;

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
    

    
    
    vec3 pos = model[3].xyz;
    vec3 dif = abs(camPos-pos);
    vec3 scale = vec3(model[0][0], model[1][1], model[2][2]);
    bool inside = min(dif, abs(scale) / 2.0) == dif;
    vec3 p = aPos * (float(!inside) * 2.0 - 1.0);

    vec4 calcP = model * vec4(p, 1.0);

    gl_Position = VP * calcP;
    fragPos = calcP.xyz;

    instanceID = gl_InstanceID;
}  