#version 460 core
layout (location = 0) in vec3 aPos;

out vec3 fragPos;
flat out int instanceID;

struct Camera {
    mat4 vp;
    vec4 pos_near;
    vec4 dir_far;
    vec4 ortho;
    vec2 aspectRatio_projection;
};

layout(std140, binding=1) uniform Light {
    vec4 lightColor; 
    vec4 ambientColor;
    Camera lightCam;
};

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
    float uvOffset;
    int order;
    //vec4 filler;
};

layout(std430, binding=0) buffer branch_data {
   BranchData branchs[];
};



void main()
{
    mat4 model = branchs[gl_InstanceID].model;

    vec4 calcP = model * vec4(aPos, 1.0);

    gl_Position = lightCam.vp * calcP;
    fragPos = calcP.xyz;

    instanceID = gl_InstanceID;
}  