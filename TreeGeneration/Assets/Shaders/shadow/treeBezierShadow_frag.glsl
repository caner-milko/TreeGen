#version 460 core
out float gl_FragDepth;
in vec3 fragPos;

flat in int instanceID;
flat in int inside;
#define SHOW_BBOX 0


#define MAX_STEPS 50.0
#define MIN_DIST 0.001
#define PI 3.14159265359

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
    float uvOffset;
    int order;
    //vec4 filler;
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
    int order;
};

layout(std430, binding=0) buffer branch_data {
    BranchData branchs[];
};

uniform vec3 camPos;
uniform vec3 viewDir;
uniform float farPlane, nearPlane;
uniform mat4 VP;
struct Hit {
	float t;
    bool hit;
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
                             bData.uvOffset,
                             bData.order);
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

float easeOutQuad(float x){
return 1.0 - (1.0 - x) * (1.0 - x);
}

float ease(float x) {
    return x;mix(easeInOutSine(x), x, 0.25);
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

#if 0
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
#else

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
    vec2 clampedT = clamp(t, 0.0, 1.0);

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

#endif

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

Hit intersect(vec3 pos, vec3 rayDir, in Branch branch) {
	float t = 0.0;
    Bezier curve = toBezier(branch);
	bool onSphere = false;
    bool hit = false;
    for(float i = 0; i < MAX_STEPS; i++) {
		vec3 curPos = pos + rayDir * t;
        vec2 dst = dist(curPos, curve);
        t += dst.x;
		if(dst.x < MIN_DIST) {
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
    vec3 rayDir = normalize(-viewDir);

    BranchData bData = branchs[instanceID];

    Branch branch = toBranch(bData);

    vec3 start = fragPos;

	Hit hit = intersect(start, rayDir, branch);
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

    vec4 posLightSpace = VP * vec4(pos, 1.0);

    vec3 projCoords = posLightSpace.xyz / posLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5; 

    gl_FragDepth = projCoords.z;
    return;
} 