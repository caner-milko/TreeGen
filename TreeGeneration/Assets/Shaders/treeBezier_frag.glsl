#version 460 core
out vec4 FragColor;
layout (depth_greater) out float gl_FragDepth;
in vec3 fragPos;

flat in int instanceID;
flat in int inside;
#define SHOW_BBOX 0


#define MAX_STEPS 50.0
#define MIN_DIST 0.0001
#define PI 3.14159265359

#define NORMAL_T 0.2

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

uniform vec3 treeColor;

uniform sampler2D shadowMap;

struct Material {
    sampler2D colorTexture;
    sampler2D normalTexture;
};

uniform Material treeMaterial;

struct Hit {
	float t;
    vec2 uv;
    float splineT;
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

vec2 calc_uv(Branch branch, float clampedT, vec3 curPos, float v) {
    vec2 uv = vec2(calc_uvx(branch, clampedT, curPos), v);
    uv.x = mod(uv.x + PI * 2.0, PI * 2.0) / PI / 2.0;
    uv.y = branch.startLength + uv.y * branch.branchLength;
    uv.y *= 20.0f;
    
    return uv;
}

vec3 bezNorm(float splineT, vec3 pos, Bezier curve) {
    vec3 bezPos = bezier(curve, splineT);
    return normalize(pos - bezPos);
}

Hit intersect(vec3 pos, vec3 rayDir, in Branch branch) {
	float t = 0.0;
	vec3 norm = vec3(-3.0);
    vec2 uv = vec2(0.0);
    Bezier curve = toBezier(branch);
	bool onSphere = false;
    float farPlane = cam.dir_far.w;
    float splineT = -100.0;
    for(float i = 0; i < MAX_STEPS; i++) {
		vec3 curPos = pos + rayDir * t;
        vec2 dst = dist(curPos, curve);
        t += dst.x;
		if(dst.x < MIN_DIST) {
            norm = vec3(-2.0);
            curPos = pos + rayDir * t;
            splineT = dst.y;
			break;
		}
		if(t > farPlane) {
			break;
		}
	}

    if(splineT != -100.0) {
		vec3 curPos = pos + rayDir * t;
        float clampedT = clamp(splineT, 0.0, 1.0);
        uv = calc_uv(branch, clampedT, curPos, splineT);
        
        //all works
        //norm = normalize(cross(dFdx(curPos), dFdy(curPos)));
        //norm = normal(curPos, curve);
        norm = bezNorm(clampedT, curPos, curve);

        norm = calcNorm(curPos, clampedT, NORMAL_T, norm, curve);
            
        onSphere = splineT > 1.0 || splineT < 0.0;
        splineT = clampedT;
    }

	return Hit(t, uv, splineT, norm, onSphere);
}


float diffuse(vec3 normal, vec3 lightDir) {
    return max(dot(normal, -lightDir), 0.0);
}

float specular(vec3 normal, vec3 viewDir, vec3 lightDir, float specularStrength, int shininess) {
    vec3 halfwayDir = normalize(-lightDir + viewDir);

    float spec = pow(max(dot(halfwayDir, normal), 0.0), shininess);
    return spec * specularStrength;
}

vec3 calcLight(vec3 viewDir, vec3 normal, vec3 col) {
    vec3 lightDir = lightCam.dir_far.xyz;
    float diff = diffuse(normal, lightDir);

    float spec = specular(normal, viewDir, lightDir, 3.0, 64);
    vec3 light = (spec + diff) * lightColor.xyz + ambientColor.xyz;

    vec3 color = light * col;

    return color;
}



float calcShadow(vec3 pos) {
    vec2 poissonDisk[4] = vec2[](
        vec2( -0.94201624, -0.39906216 ),
        vec2( 0.94558609, -0.76890725 ),
        vec2( -0.094184101, -0.92938870 ),
        vec2( 0.34495938, 0.29387760 )
    );
    vec4 posLightSpace = lightCam.vp * vec4(pos, 1.0);

    vec3 projCoords = posLightSpace.xyz / posLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5; 

    float bias = 0.001;
    float visibility = 1.0;
    for (int i=0;i<4;i++){
            visibility -= 0.2 * float((projCoords.z - bias)  > texture(shadowMap, projCoords.xy + poissonDisk[i]/700.0).r);
    }

    return visibility;
}

vec3 bumpMap(in Bezier curve, vec3 normTexture, in vec3 curNorm, in vec3 pos, in float splineT, in float strength) {
    vec3 N = curNorm;
    vec3 B = normalize(bezier_dx(curve, splineT));
    B = normalize(B - dot(B, N) * N);
    vec3 T = -normalize(cross(B, N));

    mat3 TBN = mat3(T, B, N);

    vec3 bumpMap = 2.0 * normTexture - vec3(1.0, 1.0, 1.0);

    bumpMap.z /= strength;

    vec3 norm = normalize(TBN * bumpMap);

    return norm;
}


void main()
{
    vec3 rayDir = normalize(fragPos - cam.pos_near.xyz);

    BranchData bData = branchs[instanceID];

    Branch branch = toBranch(bData);

    vec3 start = mix(fragPos, cam.pos_near.xyz, float(inside));

	Hit hit = intersect(start, rayDir, branch);
	if(hit.normal.x < -1.5) {
        #if SHOW_BBOX

        FragColor = vec4(vec3(float(gl_FrontFacing)), 1.0);
        gl_FragDepth = 0.999;
        
        return;
        #else
		discard;
        return;
	    #endif
    }

    hit.t += float(inside == 0) * length(cam.pos_near.xyz - fragPos);

    float dist = hit.t + float(hit.onSphere) * branch.highRadius / 10.0;

    float eyeHitZ = -dist * dot(cam.dir_far.xyz, rayDir);

    float farPlane = cam.dir_far.w;
    float nearPlane = cam.pos_near.w;

    float ndcDepth = ((farPlane + nearPlane) + (2.0 * farPlane * nearPlane) / eyeHitZ) / (farPlane - nearPlane);

    gl_FragDepth =((gl_DepthRange.diff * ndcDepth) 
		+ gl_DepthRange.near + gl_DepthRange.far) / 2.0;

    vec3 pos = cam.pos_near.xyz + rayDir * hit.t;
    
    float order = easeOutQuad(min(float(branch.order), 100.0)/100.0);
    float nextOrder = easeOutQuad(min(float(branch.order+1), 100.0)/100.0);

    float mixedOrder = mix(0.2, 1.0, mix(order, nextOrder, hit.splineT));
    vec3 col = texture(treeMaterial.colorTexture, hit.uv).xyz * mixedOrder;
    
    Bezier curve = toBezier(branch);
    
    vec3 norm = bumpMap(curve, texture(treeMaterial.normalTexture, hit.uv).xyz, hit.normal, pos, hit.splineT, 5.0);

    vec3 color = calcLight(-rayDir, norm, col) * calcShadow(pos);
    //vec3 color = vec3(calcShadow(pos));

    FragColor = vec4(color, 1.0);
    //FragColor = vec4(norm, 1.0);
    //FragColor = vec4(floor(hit.uv.x*4.0)/4.0, 0.0, 0.0, 1.0);
    return;
} 