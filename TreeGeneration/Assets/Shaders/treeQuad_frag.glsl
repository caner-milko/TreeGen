#version 330 core
out vec4 FragColor;

in vec3 fragPos;
in vec2 uv;

struct Branch {
	vec3 start;
	vec3 end;
	float lowRadius;
	float highRadius;
	vec3 camProjected;
	vec3 camCross;
};
uniform vec3 camPos;

uniform Branch branch;

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

struct Hit
{
    float t;
    vec3 normal;
};

const Hit noHit = Hit(-1.0, vec3(1.0, 0.0, 0.0));

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

    float lineLen = length(branch.end - branch.start);

    float d = aCone.height - lineLen;

    if(h < d) return noHit;

    vec3 n = normalize(cp * dot(aCone.axis, cp) / dot(cp, cp) - aCone.axis);

    return Hit(t, n);
}

AngleCone truncatedToAngleCone() {
    float lineLen = length(branch.end - branch.start);
    
    AngleCone aCone;
    vec3 lineDir = normalize(branch.end - branch.start);

    if(branch.highRadius == 0.0f) {
        aCone.height = lineLen;
        aCone.cosa = lineLen / sqrt(branch.lowRadius*branch.lowRadius + lineLen * lineLen);
        aCone.tip = branch.end;
        aCone.axis = -lineDir;
        return aCone;
    }
    float d = branch.highRadius * lineLen / (branch.lowRadius - branch.highRadius);
    



    aCone.cosa = d/sqrt(d*d + branch.highRadius * branch.highRadius);
    aCone.height = d + lineLen;

    aCone.tip = branch.start + lineDir * aCone.height;

    aCone.axis = -lineDir;
    return aCone;
}

void main()
{
    vec3 rayDir = normalize(fragPos - camPos);
    vec3 mid = (branch.end + branch.start)/2.0;
    float rad = max(branch.lowRadius, branch.highRadius);

    AngleCone aCone = truncatedToAngleCone();

    Hit intersection = intersectRayCone(camPos, rayDir, aCone);

    if(intersection.t == -1.0) {
        //FragColor = vec4(0.0, 0.0, 0.0, 1.0);
        //return;
        discard;
    }

    vec3 pos = camPos + rayDir * intersection.t;

    vec3 col = treeColor * (dot(intersection.normal, lightDir) * lightColor + ambientColor);

    FragColor = vec4(col, 1.0);
} 