#version 330 core
out vec4 FragColor;
out float gl_FragDepth;
in vec3 fragPos;
in vec2 uv;

#define MAX_DIST 40.0

struct Branch {
	vec3 start;
	vec3 end;
	float lowRadius;
	float highRadius;
	vec3 camProjected;
	vec3 camCross;
};
uniform vec3 camPos;

uniform Branch branchMain;

uniform vec3 treeColor;

uniform vec3 ambientColor;
uniform vec3 lightDir;
uniform vec3 lightColor;

struct AngleCone
{
    float cosa;	// half cone angle
    float height;	// height
    vec3 tip;		// tip position
    vec3 axis;		// axis
};

struct Sphere 
{
    vec3 pos;
    float radius;
};

struct Hit
{
    float t;
    vec3 normal;
};

const Hit noHit = Hit(-1.0, vec3(1.0, 0.0, 0.0));

Hit intersectRaySphere(vec3 rayOrigin, vec3 rayDir, Sphere sphere) {
    vec3 oc = rayOrigin - sphere.pos;
    float b = dot( oc, rayDir );
    float c = dot( oc, oc ) - sphere.radius*sphere.radius;
    float h = b*b - c;
    if( h<0.0 ) return noHit; // no intersection
    h = sqrt(h);
    float t = min(-b-h, -b+h);
    vec3 point = rayOrigin + t * rayDir;
    return Hit(t, normalize(point - sphere.pos));
}

// https://lousodrome.net/blog/light/2017/01/03/intersection-of-a-ray-and-a-cone/
Hit intersectRayCone(vec3 rayOrigin, vec3 rayDir, AngleCone aCone) {
    vec3 co = rayOrigin - aCone.tip;

    float a = dot(rayDir, aCone.axis) * dot(rayDir, aCone.axis) - aCone.cosa * aCone.cosa;
    float b = 2. * (dot(rayDir,aCone.axis) * dot(co , aCone.axis) - dot(rayDir, co) * aCone.cosa * aCone.cosa);
    float c = dot(co, aCone.axis) * dot(co, aCone.axis) - dot(co, co) * aCone.cosa * aCone.cosa;

    float det = b * b - 4. * a * c;
    if (det < 0.) return noHit;

    det = sqrt(det);
    float t1 = (-b - det) / (2. * a);
    float t2 = (-b + det) / (2. * a);

    // This is a bit messy; there ought to be a more elegant solution.
    float t = t1;
    if (t < 0. || t2 > 0. && t2 < t) t = t2;
    if (t < 0.) return noHit;

    vec3 cp = rayOrigin + t * rayDir - aCone.tip;
    float h = dot(cp, aCone.axis);
    if (h < 0. || h > aCone.height) return noHit;

    float lineLen = length(branchMain.end - branchMain.start);

    float d = aCone.height - lineLen;

    if(h < d) return noHit;

    vec3 n = normalize(cp * dot(aCone.axis, cp) / dot(cp, cp) - aCone.axis);

    return Hit(t, n);
}

AngleCone truncatedToAngleCone() {
    float lineLen = length(branchMain.end - branchMain.start);
    
    AngleCone aCone;
    vec3 lineDir = normalize(branchMain.end - branchMain.start);

    if(branchMain.highRadius == 0.0f) {
        aCone.height = lineLen;
        aCone.cosa = lineLen / sqrt(branchMain.lowRadius*branchMain.lowRadius + lineLen * lineLen);
        aCone.tip = branchMain.end;
        aCone.axis = -lineDir;
        return aCone;
    }
    float d = branchMain.highRadius * lineLen / (branchMain.lowRadius - branchMain.highRadius);
    



    aCone.cosa = d/sqrt(d*d + branchMain.highRadius * branchMain.highRadius);
    aCone.height = d + lineLen;

    aCone.tip = branchMain.start + lineDir * aCone.height;

    aCone.axis = -lineDir;
    return aCone;
}

void main()
{
    vec3 rayDir = normalize(fragPos - camPos);
    vec3 mid = (branchMain.end + branchMain.start)/2.0;
    float rad = max(branchMain.lowRadius, branchMain.highRadius);

    AngleCone aCone = truncatedToAngleCone();

    Sphere sphere = Sphere(branchMain.end, branchMain.highRadius);

    Hit intersection = intersectRayCone(camPos, rayDir, aCone);
    Hit sphereIntersection = intersectRaySphere(camPos, rayDir, sphere);
    if(intersection.t == -1.0 || (sphereIntersection.t < intersection.t && sphereIntersection.t >= 0.0))
        intersection = sphereIntersection;
    
    if(intersection.t == -1.0) {
        //FragColor = vec4(0.0, 0.0, 0.0, 1.0);
        //return;
        discard;
    }

    

    vec3 pos = camPos + rayDir * intersection.t;

    vec3 col = treeColor * (dot(intersection.normal, lightDir) * lightColor + ambientColor);
    gl_FragDepth = intersection.t / MAX_DIST;
    FragColor = vec4(col, 1.0);
} 