#version 460 core
layout (location = 0) in vec3 aPos;

out vec3 fragPos;
flat out int instanceID;
flat out int inside;

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
    
    vec3 camPos = cam.pos_near.xyz;
    
    
    vec3 pos = model[3].xyz;
    vec3 dif = abs(camPos-pos);
    vec3 scale = vec3(model[0][0], model[1][1], model[2][2]);
    bool inside = min(dif, abs(scale) / 2.0) == dif;
    vec3 p = aPos * (float(!inside) * 2.0 - 1.0);

    vec4 calcP = model * vec4(p, 1.0);

    gl_Position = cam.vp * calcP;
    fragPos = calcP.xyz;

    instanceID = gl_InstanceID;
}  