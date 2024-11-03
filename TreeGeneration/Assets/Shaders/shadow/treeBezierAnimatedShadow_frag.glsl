#version 460 core
out float gl_FragDepth;
in vec3 fragPos;

flat in int instanceID;

//these can be adjusted based on distance to camera

#define MAX_STEPS 15.0
#define MIN_DIST 0.00001
#define PI 3.14159265359

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

struct Bezier {
    vec3 start;
    vec3 mid;
    vec3 end;
    float lowRadius;
    float highRadius;
    float endT;
};

struct AnimatedBranchData {
    mat4 model;
	vec4 start;
    vec4 mid;
	vec4 end;
    vec4 color;
	vec2 lowRadiusBounds;
	vec2 highRadiusBounds;
    vec2 TBounds;
    vec2 animationBounds;
    float startLength;
    float branchLength;
    float uvOffset;
    int order;
};

struct AnimatedBranch {
    mat4 model;
    float endT;
	vec3 start;
    vec3 mid;
	vec3 end;
    vec3 color;
	float lowRadius;
	float highRadius;
    float startLength;
    float branchLength;
    float uvOffset;
    int order;
};

layout(std430, binding=0) buffer branch_data {
    AnimatedBranchData branchs[];
};

uniform float animationT;

uniform vec3 treeColor;

uniform sampler2DShadow shadowMap;

struct Material {
    sampler2D colorTexture;
    sampler2D normalTexture;
};

uniform Material treeMaterial;

struct Hit {
	float t;
    bool hit;
    bool onSphere;
};

float MapBoundsT(vec2 TBounds, vec2 animationTBounds, float animationT) {
    return mix(TBounds.x, TBounds.y, clamp((animationT - animationTBounds.x) / max(0.001, animationTBounds.y - animationTBounds.x), 0.0, 1.0));
}

float MapLowRadius(vec2 lowRadiusBounds, vec2 animationTBounds, float animationT) {
    return mix(lowRadiusBounds.x, lowRadiusBounds.y, clamp((animationT - animationTBounds.x) / (1.0 - animationTBounds.x), 0.0, 1.0));
}

float MapHighRadius(vec2 highRadiusBounds, vec2 animationTBounds, float animationT) {
    return mix(highRadiusBounds.x, highRadiusBounds.y, clamp((animationT - animationTBounds.y) / max(0.001, 1.0 - animationTBounds.y), 0.0, 1.0));
}

AnimatedBranch toBranch(in AnimatedBranchData bData) {
    
    float t = MapBoundsT(bData.TBounds, bData.animationBounds, animationT);
    float lowRadius = MapLowRadius(bData.lowRadiusBounds, bData.animationBounds, animationT);
    float highRadius = MapHighRadius(bData.highRadiusBounds, bData.animationBounds, animationT);

    return AnimatedBranch(bData.model,
                          t,
	                      bData.start.xyz,
                          bData.mid.xyz,
	                      bData.end.xyz,
                          bData.color.xyz,
	                      lowRadius,
	                      highRadius,
                          bData.startLength,
                          bData.branchLength,
                          bData.uvOffset,
                          bData.order);
}

Bezier toBezier(in AnimatedBranch branch) {
    return Bezier(branch.start, branch.mid, branch.end, branch.lowRadius, branch.highRadius, branch.endT);
}

//https://www.shadertoy.com/view/wtcczf

float easeOutQuad(float x){
return 1.0 - (1.0 - x) * (1.0 - x);
}

float ease(float x) {
    return x;
}

// b(t) = (1-t)^2*A + 2(1-t)t*B + t^2*C
vec3 bezier( in Bezier bez, in float t )
{
    return (1.0-t)*(1.0-t)*bez.start + 2.0*(1.0-t)*t*bez.mid + t*t*bez.end;
}
// b'(t) = 2(t-1)*A + 2(1-2t)*B + 2t*C
vec3 bezier_dx( in Bezier bez, float t )
{
    return 2.0*(t-1.0)*bez.start + 2.0*(1.0-2.0*t)*bez.mid + 2.0*t*bez.end;
}

float dot2( in vec3 v ) { return dot(v,v); }

vec2 solveCubic(float a, float b, float c) {
    float p = b - a*a / 3.0, p3 = p*p*p;
    float q = a * (2.0*a*a - 9.0*b) / 27.0 + c;
    float d = q*q + 4.0*p3 / 27.0;
    float offset = -a / 3.0;
    if(d >= 0.0) { 
        float z = sqrt(d);
        vec2 x = (vec2(z, -z) - q) / 2.0;
        vec2 uv = sign(x)*pow(abs(x), vec2(1.0/3.0));
        return vec2(offset + uv.x + uv.y);
    }

    float x = -sqrt(-27.0 / p3) * q / 2.0;
    x = sqrt(0.5+0.5*x);
    
    float m = x*(x*(x*(x*-0.008978+0.039075)-0.107071)+0.576974)+0.5;//;cos(acos(x)/3.);//(x * x * ( 0.01875324 * x - 0.08179158 ) + ( 0.33098754 * x + 1.7320508 ))/2.0;
    float n = sqrt(1.-m*m)*sqrt(3.);
    return vec2(m + m, -n - m) * sqrt(-p / 3.0) + offset;
}

vec2 sdBezier(vec3 p, Bezier bezier)
{    
    vec3 A = bezier.start;
    vec3 B = bezier.mid;
    vec3 C = bezier.end;
    vec3 a = B - A;
    vec3 b = A - B * 2.0 + C;
    vec3 c = a * 2.0;
    vec3 d = A - p;
    
    vec3 k = vec3(3.*dot(a,b),2.*dot(a,a)+dot(d,b),dot(d,a)) / dot(b,b);     
    
    
    vec2 t = solveCubic(k.x, k.y, k.z);
    vec2 clampedT = clamp(t, 0.0, bezier.endT);

    vec3 pos = A + (c + b * clampedT.x) * clampedT.x;
    float tt = t.x;
    float dis = length(pos - p);
    pos = A + (c + b * clampedT.y) * clampedT.y;
    float dis2 = length(pos - p);
    bool y = dis2 < dis;
    dis = mix(dis, dis2, y);
    tt = mix(t.x, t.y, y);
    //pos = A + (c + b*t.z)*t.z;
    //dis = min(dis, length(pos - p));
    return vec2(dis, tt);
}

float calcStepCount(float distToCam, float nearPlane, float farPlane, float maxStepCount, float minStepCount) {
    return mix(maxStepCount, minStepCount, (distToCam - nearPlane) / (farPlane - nearPlane));
}

float calcMinDist(float distToCam, float nearPlane, float farPlane, float minMinDist, float maxMinDist) {
    return mix(minMinDist, maxMinDist, (distToCam - nearPlane) / (farPlane - nearPlane));
}

float branchRadius(in float t, in float endT, in  float lowRadius, in float highRadius) {
    t = ease(t);
    return ((endT-t) * lowRadius + t * highRadius);
}

vec2 dist(in vec3 pos, in Bezier bezier) {
    vec2 sdBranch = sdBezier(pos, bezier);

    float clampedT = clamp(sdBranch.y, 0.0, bezier.endT);
    float midDist = sdBranch.x - branchRadius(clampedT, bezier.endT, bezier.lowRadius, bezier.highRadius);
    return vec2(midDist, sdBranch.y);
}

Hit intersect(vec3 pos, vec3 rayDir, in AnimatedBranch branch, float maxStepCount, float minDist) {
    float t = 0.0;
    Bezier curve = toBezier(branch);
	bool onSphere = false;
    float farPlane = lightCam.dir_far.w;
    bool hit = false;

    for(float i = 0; i < maxStepCount; i++) {
		vec3 curPos = pos + rayDir * t;
        vec2 dst = dist(curPos, curve);
        t += dst.x;
		if(dst.x < minDist) {
            curPos = pos + rayDir * t;
            hit = true;
            onSphere = dst.y > 1.0 || dst.y < 0.0;
			break;
		}
		if(t > farPlane) {
			break;
		}
	}
	return Hit(t, hit, onSphere);
}

void main()
{
    vec3 rayDir = normalize(-lightCam.dir_far.xyz);

    AnimatedBranchData bData = branchs[instanceID];

    AnimatedBranch branch = toBranch(bData);

    if(branch.lowRadius == 0) {
        discard;
    }
    
    vec3 start = fragPos;

    float distToCam = distance(lightCam.pos_near.xyz, branch.mid);
    float stepCount = calcStepCount(distToCam, lightCam.pos_near.w, lightCam.dir_far.w, MAX_STEPS, 1.5);
    float minDist = calcMinDist(distToCam, lightCam.pos_near.w, lightCam.dir_far.w, MIN_DIST, 50.0*MIN_DIST);
    
	Hit hit = intersect(start, rayDir, branch, stepCount, minDist);
	if(!hit.hit) {
        #if SHOW_BBOX

        FragColor = vec4(vec3(float(gl_FrontFacing)), 1.0);
        gl_FragDepth = 0.999;
        
        return;
        #else
		discard;
        return;
	    #endif
    }
    
    hit.t += float(hit.onSphere) * branch.highRadius / 10.0;

    vec3 pos = fragPos + rayDir * hit.t;

    vec4 posLightSpace = lightCam.vp * vec4(pos, 1.0);

    vec3 projCoords = posLightSpace.xyz / posLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5; 

    gl_FragDepth = projCoords.z;
    return;
} 