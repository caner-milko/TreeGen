#version 460 core
out vec4 FragColor;
layout (depth_greater) out float gl_FragDepth;
in vec3 fragPos;

flat in int instanceID;

#define SHOW_BBOX 0


#define MAX_STEPS 50.0
#define MIN_DIST 0.0001
#define PI 3.14159265359

#define NORMAL_T 0.2

struct Bezier {
    vec3 start;
    vec3 mid;
    vec3 end;
    float lowRadius;
    float highRadius;
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
    vec4 uvOffset;
};

struct Branch {
    mat4 model;
	vec3 start;
    vec3 mid;
	vec3 end;
    vec3 color;
	float lowRadius;
	float highRadius;
    float startLength;
    float branchLength;
    float uvOffset;
};

layout(std430, binding=0) buffer branch_data {
    BranchData branchs[];
};

uniform vec3 camPos;
uniform float farPlane, nearPlane;

uniform vec3 treeColor;

uniform vec3 ambientColor;
uniform vec3 lightDir;
uniform vec3 lightColor;

uniform sampler2D barkTexture;

struct Hit {
	float t;
    vec2 uv;
	vec3 normal;
    bool onSphere;
};

float smin( float a, float b, float k )
{
    float h = clamp( 0.5+0.5*(b-a)/k, 0.0, 1.0 );
    return mix( b, a, h ) - k*h*(1.0-h);
}

Branch toBranch(in BranchData bData) {
    return Branch( bData.model,
	                         bData.start.xyz,
                             bData.mid.xyz,
	                         bData.end.xyz,
                             bData.color.xyz,
	                         bData.lowRadius,
	                         bData.highRadius,
                             bData.startLength,
                             bData.branchLength,
                             bData.uvOffset.x);
}

Bezier toBezier(in Branch branch) {
    return Bezier(branch.start, branch.mid, branch.end, branch.lowRadius, branch.highRadius);
}

//https://www.shadertoy.com/view/wtcczf
float easeInOutSine(float x)
{ 
    return -(cos(PI * x) - 1.0) / 2.0;
}

float easeInOutQuad(float x)
{
   //x < 0.5f ? 2 * x* x : 1 - pow(-2 * x + 2,2) /2;
   float inValue = 2.0 * x  *x;
   float outValue = 1.0- pow(-2.0 * x + 2.0,2.0) / 2.0;
   float inStep = step(inValue,0.5) * inValue;
   float outStep = step(0.5 , outValue ) * outValue;
   
   return inStep + outStep;
}

float ease(float x) {
    return mix(easeInOutSine(x), x, 0.75);
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

vec2 sdBezier(vec3 pos, Bezier bezier)
{    
    vec3 A = bezier.start;
    vec3 B = bezier.mid;
    vec3 C = bezier.end;
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
        float t = uv.x+uv.y-kx;

        float clampedT = clamp(t, 0.0, 1.0);

        // 1 root
        res = vec2(dot2(d+(c+b*clampedT)*clampedT),t);

    }
    else
    {
        float z = sqrt(-p);
        float v = acos( q/(p*z*2.0) ) / 3.0;
        float m = cos(v);
        float n = sin(v)*1.732050808;
        vec3 t = vec3(m+m,-n-m,n-m)*z-kx;
        
        vec3 clampedT = clamp(t, 0.0, 1.0);

        // 3 roots, but only need two
        float dis = dot2(d+(c+b*clampedT.x)*clampedT.x);
        res = vec2(dis,t.x);

        dis = dot2(d+(c+b*clampedT.y)*clampedT.y);
        if( dis<res.x ) res = vec2(dis,t.y );
    }
    
    res.x = sqrt(res.x);
    return res;
}

float branchRadius(in float t, in  float lowRadius, in float highRadius) {

    t = ease(t);

    return ((1.-t)*lowRadius + t * highRadius);
}

vec2 dist(in vec3 pos, in Bezier bezier) {
    vec2 sdBranch = sdBezier(pos, bezier);

    float clampedT = clamp(sdBranch.y, 0.0, 1.0);
    float midDist = sdBranch.x - branchRadius(clampedT, bezier.lowRadius, bezier.highRadius);
    return vec2(midDist, sdBranch.y);
}

vec2 smoothDist(in vec3 pos) {
    Branch main = toBranch(branchs[instanceID]);
    vec2 a = dist(pos, toBezier(main));
    float dst = a.x;
    const float k = 0.001;
    const float offset = 0.00005;

    if(instanceID > 0) {
        Bezier beforeBez = toBezier(toBranch(branchs[instanceID-1]));
        if(beforeBez.end == main.start) {
            a = dist(pos, beforeBez);
            dst = smin(dst, dist(pos, beforeBez).x + offset, k);
        }
    }
    Bezier afterBez = toBezier(toBranch(branchs[instanceID+1]));
    if(afterBez.start == main.end) {
        a = dist(pos, afterBez);
        dst = smin(dst, a.x + offset, k);
    }

    return vec2(dst, a.y);
}

vec3 normal(vec3 pos, in Bezier bezier) {
	const vec2 e = vec2(MIN_DIST, 0.);
    return normalize(vec3(dist(pos + e.xyy, bezier).x - dist(pos - e.xyy, bezier).x,
              			    dist(pos + e.yxy, bezier).x - dist(pos - e.yxy, bezier).x,
              			    dist(pos + e.yyx, bezier).x - dist(pos - e.yyx, bezier).x));
}

vec3 smoothNormal(in vec3 pos) {
	const vec2 e = vec2(MIN_DIST, 0.);
    return normalize(vec3(smoothDist(pos + e.xyy).x - smoothDist(pos - e.xyy).x,
              			    smoothDist(pos + e.yxy).x - smoothDist(pos - e.yxy).x,
              			    smoothDist(pos + e.yyx).x - smoothDist(pos - e.yyx).x));
}

vec3 coneNormal(in vec3 pos, in float t, in vec3 A, in vec3 B, in float Arad, in float Brad) {
    vec3 AP = normalize(pos-A);
    vec3 BP = normalize(pos-B);
    vec3 dif = AP;
    vec3 AB = normalize(B-A);
    vec3 Acone = normalize(AP - dot(AB, AP) * AB) * Arad + A;
    vec3 Bcone = normalize(BP - dot(AB, BP) * AB) * Brad + B;

    vec3 coneDif = normalize(Bcone-Acone);

    return normalize(AP - dot(AP, coneDif) * coneDif);

}

vec3 calcNorm(in vec3 pos, in float t, float tDif, in vec3 normal, in Bezier main) {
    if(t < tDif) {
        vec3 mid = bezier(main, tDif);
        float widthAtTdif = branchRadius(tDif, main.lowRadius, main.highRadius);
        vec3 coneNorm = coneNormal(pos, t, main.start, mid, main.lowRadius, widthAtTdif);
        return normalize(mix(normal, coneNorm, (tDif - t)/tDif));
    } else if(t > (1.0 - tDif)) {
        Bezier bez = toBezier(toBranch(branchs[instanceID + 1]));
        if(bez.start == main.end) {
            vec3 mid = bezier(bez, tDif);
            float widthAtTdif = branchRadius(tDif, bez.lowRadius, bez.highRadius);
            vec3 coneNorm = coneNormal(pos, t, bez.start, mid, bez.lowRadius, widthAtTdif);
            return normalize(mix(normal, coneNorm, (t - (1.0 - tDif))/tDif));
        } else {
            return normal;
        }
    }
    else {
        return normal;
    }
}

float calc_uvx(Branch branch, float clampedT, vec3 curPos) {
    Bezier curve = toBezier(branch);
    vec3 bezierPos = bezier(curve, clampedT);
    vec3 bezierPlaneNormal = normalize(cross(curve.mid - curve.start, curve.end - curve.start));
    vec3 bezierDir = normalize(bezier_dx(curve, clampedT));
    vec3 bezierNormalOnPlane = normalize(cross(bezierPlaneNormal,bezierDir));
    vec2 v = vec2(dot(curPos-bezierPos,bezierNormalOnPlane),dot(curPos-bezierPos,bezierPlaneNormal));

    return atan(v.y,v.x) + branch.uvOffset.x;
}

Hit intersect(vec3 pos, vec3 rayDir, in Branch branch) {
	float t = 0.0;
	vec3 norm = vec3(-2.0);
    vec2 uv = vec2(0.0);
    Bezier curve = toBezier(branch);
	bool onSphere = false;
    for(float i = 0; i < MAX_STEPS; i++) {
		vec3 curPos = pos + rayDir * t;
        vec2 dst = dist(curPos, curve);
        t += dst.x;
		if(dst.x < MIN_DIST) {
            curPos = pos + rayDir * t;
            float clampedT = clamp(dst.y, 0.0, 1.0);
            
            //ka = calc_uvx(curve, clampedT, curPos);
            uv = vec2(calc_uvx(branch, clampedT, curPos), dst.y);
            uv.x = mod(uv.x, PI * 2.0) / PI / 2.0;
            uv.y = branch.startLength + uv.y * branch.branchLength;
            uv.y *= 20.0f;

			norm = normal(curPos, curve);

            norm = calcNorm(curPos, clampedT, NORMAL_T, norm, curve);
            
            
            onSphere = dst.y > 1.0 || dst.y < 0.0;
			break;
		}
		if(t > farPlane) {
			break;
		}
	}
	return Hit(t, uv, norm, onSphere);
}

bool test() {
    Branch main = toBranch(branchs[instanceID]);
    Branch bef = toBranch(branchs[instanceID-1]);
    if(bef.end !=main.start) {
        return false;
    }

    Bezier mainBez = toBezier(main);
    Bezier befBez = toBezier(bef);

    vec3 ABm = normalize(mainBez.mid- mainBez.start);
    vec3 ACm = normalize(mainBez.mid- mainBez.start);

    vec3 ABb = normalize(befBez.mid- befBez.start);
    vec3 ACb = normalize(befBez.mid- befBez.start);

    vec3 bezierPlaneNormal = normalize(cross(mainBez.mid - mainBez.start, mainBez.end - mainBez.start));

    vec3 befNormal = normalize(cross(befBez.mid - befBez.start, befBez.end - befBez.start));

    vec3 bezierDir = normalize(bezier_dx(mainBez, 0.0));

    vec3 bezierNormalOnPlane = normalize(cross(bezierPlaneNormal,bezierDir));

    vec3 bBezierDir = normalize(bezier_dx(befBez, 1.0));

    vec3 bBezierNormalOnPlane = normalize(cross(befNormal,bBezierDir));

    return cross(bezierPlaneNormal, befNormal).x > 0.0;

}

void main()
{
    vec3 rayDir = normalize(fragPos - camPos);

    BranchData bData = branchs[instanceID];

    Branch branch = toBranch(bData);

	Hit hit = intersect(fragPos, rayDir, branch);
    
	if(hit.normal.x < -1.5) {
        #if SHOW_BBOX
        FragColor = vec4(vec3(1.0), 1.0);
        gl_FragDepth = 0.999;
        #else
		discard;
        return;
	    #endif
    }
    hit.t += length(camPos-fragPos);

    float dist = (1.0/(hit.t + float(hit.onSphere) * branch.highRadius / 10.0)  - 1.0/nearPlane)/(1.0/farPlane - 1.0/nearPlane);
     
    gl_FragDepth = dist;

    float d = clamp(dot(hit.normal, -lightDir), 0.0, 1.0);
    vec3 light = d * lightColor + ambientColor;
    vec4 color = vec4(light, 1.0f) * texture(barkTexture, hit.uv);
    FragColor = pow(color, vec4(1.0/2.2));
    //FragColor = vec4(0.0, floor(hit.uv.x*4.0)/4.0, 0.0, 1.0);
    return;
} 