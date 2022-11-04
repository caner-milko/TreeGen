﻿#version 330 core
out vec4 FragColor;
out float gl_FragDepth;
in vec3 fragPos;
in vec2 uv;

#define MAX_STEPS 150.0
#define MAX_DIST 40.0
#define MIN_DIST 0.00001

struct Branch {
	vec3 start;
	vec3 end;
	float lowRadius;
	float highRadius;
	vec3 camProjected;
	vec3 camCross;
};
uniform vec3 camPos;

uniform Branch branchMain, branchBefore, branchNext;

uniform vec3 treeColor;

uniform vec3 ambientColor;
uniform vec3 lightDir;
uniform vec3 lightColor;

struct Hit {
	float t;
	vec3 normal;
};

float dot2( in vec3 v ) { return dot(v,v); }

vec2 sdBezier(vec3 pos, vec3 A, vec3 B, vec3 C)
{    
    vec3 a = B - A;
    vec3 b = A - 2.0*B + C;
    vec3 c = a * 2.0;
    vec3 d = A - pos;

    float kk = 1.0 / dot(b,b);
    float kx = kk * dot(a,b);
    float ky = kk * (2.0*dot(a,a)+dot(d,b)) / 3.0;
    float kz = kk * dot(d,a);      

    vec2 res;

    float p = ky - kx*kx;
    float p3 = p*p*p;
    float q = kx*(2.0*kx*kx - 3.0*ky) + kz;
    float q2 = q*q;
    float h = q2 + 4.0*p3;

    if(h >= 0.0) 
    { 
        h = sqrt(h);
        vec2 x = (vec2(h, -h) - q) / 2.0;
        
        #if 1
        // When p≈0 and p<0, h-q has catastrophic cancelation. So, we do
        // h=√(q²+4p³)=q·√(1+4p³/q²)=q·√(1+w) instead. Now we approximate
        // √ by a linear Taylor expansion into h≈q(1+½w) so that the q's
        // cancel each other in h-q. Expanding and simplifying further we
        // get x=vec2(p³/q,-p³/q-q). And using a second degree Taylor
        // expansion instead: x=vec2(k,-k-q) with k=(1-p³/q²)·p³/q
        if( abs(p)<0.001 )
        {
          //float k = p3/q;              // linear approx
            float k = (1.0-p3/q2)*p3/q;  // quadratic approx 
            x = vec2(k,-k-q);  
        }
        #endif
        
        vec2 uv = sign(x)*pow(abs(x), vec2(1.0/3.0));
        float t = clamp(uv.x+uv.y-kx, 0.0, 1.0);

        // 1 root
        res = vec2(dot2(d+(c+b*t)*t),t);
        
        //res = vec2( dot2( pos-bezier(A,B,C,t)), t );
    }
    else
    {
        float z = sqrt(-p);
        float v = acos( q/(p*z*2.0) ) / 3.0;
        float m = cos(v);
        float n = sin(v)*1.732050808;
        vec3 t = clamp( vec3(m+m,-n-m,n-m)*z-kx, 0.0, 1.0);
        
        // 3 roots, but only need two
        float dis = dot2(d+(c+b*t.x)*t.x);
        res = vec2(dis,t.x);

        dis = dot2(d+(c+b*t.y)*t.y);
        if( dis<res.x ) res = vec2(dis,t.y );
    }
    
    res.x = sqrt(res.x);
    return res;
}

vec2 sdBranch(vec3 pos, Branch branch) {
    vec3 mid = (branch.start + branch.end) /2.0 + vec3( 0.005f, 0.005f, 0.005f);
    return sdBezier(pos, branch.start, mid, branch.end);
}

float branchRadius(Branch branch, float t) {
    return ((1.-t)*branch.lowRadius + t * branch.highRadius);
}

float dist(vec3 pos) {
    vec2 sdBranchBefore = sdBranch(pos, branchBefore);
    vec2 sdBranchMid = sdBranch(pos, branchMain);
    vec2 sdBranchNext = sdBranch(pos, branchNext);

    if(branchBefore.lowRadius < 0.0) {
        sdBranchBefore.x = MAX_DIST * 2.;    
    }
    if(branchNext.lowRadius < 0.0) {
        sdBranchNext.x = MAX_DIST * 2.;
    }

    float beforeDist = sdBranchBefore.x - branchRadius(branchBefore, sdBranchBefore.y);

    float midDist = sdBranchMid.x - branchRadius(branchMain, sdBranchMid.y);
    
    float nextDist = sdBranchNext.x - branchRadius(branchNext, sdBranchNext.y);
    
    return min(beforeDist, min(midDist, nextDist));
}

vec3 normal(vec3 pos) {
	const vec2 e = vec2(MIN_DIST, 0.);
    return normalize(vec3(dist(pos + e.xyy) - dist(pos - e.xyy),
              			    dist(pos + e.yxy) - dist(pos - e.yxy),
              			    dist(pos + e.yyx) - dist(pos - e.yyx)));
}

Hit intersect(vec3 pos, vec3 rayDir) {
	float t = 0.0;
	vec3 norm = vec3(-2.0);
	for(float i = 0; i < MAX_STEPS; i++) {
		vec3 curPos = pos + rayDir * t;
		float dst = dist(curPos);
		t += dst;
		if(dst < MIN_DIST) {
			norm = normal(curPos + dst * rayDir);
			break;
		}
		if(t > MAX_DIST) {
			break;
		}
	}
	return Hit(t, norm);
}

void main()
{
    vec3 rayDir = normalize(fragPos - camPos);

	Hit hit = intersect(camPos, rayDir);
	if(hit.normal.x < -1.5) {
		discard;
	}
    gl_FragDepth = hit.t / MAX_DIST;
    FragColor = vec4((vec3(1.) - treeColor) * (dot(hit.normal, lightDir) * lightColor + ambientColor), 1.0);
    
} 