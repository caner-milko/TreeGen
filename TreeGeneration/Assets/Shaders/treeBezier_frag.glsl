#version 330 core
out vec4 FragColor;
out float gl_FragDepth;
in vec3 fragPos;

#define SHOW_BBOX 0


#define MAX_STEPS 150.0
#define MAX_DIST 40.0
#define MIN_DIST 0.0001
#define PI 3.14159265359


struct Branch {
	vec3 start;
	vec3 end;
    vec3 mid;
	float lowRadius;
	float highRadius;
    vec3 color;
    float branchLength;
    float startLength;
    float uvOffset;
};
uniform vec3 camPos;
uniform float farPlane, nearPlane;


uniform Branch branch;

uniform vec3 treeColor;

uniform vec3 ambientColor;
uniform vec3 lightDir;
uniform vec3 lightColor;

uniform sampler2D barkTexture;

struct Hit {
	float t;
    vec2 uv;
	vec3 normal;
};

// b(t) = (1-t)^2*A + 2(1-t)t*B + t^2*C
vec3 bezier( vec3 A, vec3 B, vec3 C, float t )
{
    return (1.0-t)*(1.0-t)*A + 2.0*(1.0-t)*t*B + t*t*C;
}
// b'(t) = 2(t-1)*A + 2(1-2t)*B + 2t*C
vec3 bezier_dx( vec3 A, vec3 B, vec3 C, float t )
{
    return 2.0*(t-1.0)*A + 2.0*(1.0-2.0*t)*B + 2.0*t*C;
}

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


float branchRadius(Branch branch, float t) {
    return ((1.-t)*branch.lowRadius + t * branch.highRadius);
}

vec2 dist(vec3 pos) {
    vec2 sdBranch = sdBezier(pos, branch.start, branch.mid, branch.end);

    float midDist = sdBranch.x - branchRadius(branch, sdBranch.y);
    return vec2(midDist, sdBranch.y);
}

vec3 normal(vec3 pos) {
	const vec2 e = vec2(MIN_DIST, 0.);
    return normalize(vec3(dist(pos + e.xyy).x - dist(pos - e.xyy).x,
              			    dist(pos + e.yxy).x - dist(pos - e.yxy).x,
              			    dist(pos + e.yyx).x - dist(pos - e.yyx).x));
}

Hit intersect(vec3 pos, vec3 rayDir) {
	float t = 0.0;
	vec3 norm = vec3(-2.0);
    vec2 uv = vec2(0.0);
	for(float i = 0; i < MAX_STEPS; i++) {
		vec3 curPos = pos + rayDir * t;
        vec2 dst = dist(curPos);
		if(dst.x < MIN_DIST) {
			norm = normal(curPos + dst.x * rayDir);

            vec3 bezierPlaneNormal = normalize(cross(branch.mid-branch.start, branch.end-branch.start));
            vec3 bezierPos = bezier(branch.start,branch.mid,branch.end,dst.y);
            vec3 bezierDir = normalize(bezier_dx(branch.start, branch.mid, branch.end, dst.y));
            vec3 bezierNormalOnPlane = normalize(cross(bezierPlaneNormal,bezierDir));
            vec2 v = vec2(dot(curPos-bezierPos,bezierNormalOnPlane),dot(curPos-bezierPos,bezierPlaneNormal));

            float ka = atan(v.y,v.x) - branch.uvOffset;

            
            uv = vec2(ka, dst.y);
            uv.x = mod(uv.x, PI * 2.0) / PI / 2.0;
            uv.y = branch.startLength + uv.y * branch.branchLength;
            uv.y *= 3.0f;
			break;
		}
        t += dst.x;
		if(t > MAX_DIST) {
			break;
		}
	}
	return Hit(t, uv, norm);
}

void main()
{
    vec3 rayDir = normalize(fragPos - camPos);

	Hit hit = intersect(camPos, rayDir);
    
	if(hit.normal.x < -1.5) {
        #if SHOW_BBOX
        FragColor = vec4(vec3(1.0), 1.0);
        gl_FragDepth = 0.999;
        #else
		discard;
        return;
	    #endif
    }

    float dist = (1.0/hit.t - 1.0/nearPlane)/(1.0/farPlane - 1.0/nearPlane);

    gl_FragDepth = dist;

    vec3 light = dot(hit.normal, lightDir) * lightColor + ambientColor;

    //hit.uv.x = floor(hit.uv.x * 4.0f) /4.0f;

    FragColor = vec4(light, 1.0f) * texture(barkTexture, hit.uv);

    return;
    
} 