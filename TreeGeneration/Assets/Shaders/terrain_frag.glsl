#version 460 core
out vec4 FragColor;

in vec3 fragPos;
in vec2 uv;
in mat3 TBN;

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

layout(std430, binding=0) buffer branch_data {
    vec4 obstacles[];
};

uniform int obstacleCount;

struct TerrainMaterial {
    sampler2D grassTex;
    sampler2D dirtTex;
    float grassColorMultiplier;
    float dirtColorMultiplier;
    sampler2D normalMap;
    float normalMapStrength;
    float uvScale;
};

uniform TerrainMaterial material;

uniform sampler2DShadow shadowMap;

float diffuse(vec3 norm, vec3 lightDir) {
    return max(dot(norm, -lightDir), 0.0);
}

float specular(vec3 norm, vec3 viewDir, vec3 lightDir, float specularStrength, int shininess) {
    vec3 halfwayDir = normalize(-lightDir + viewDir);

    float spec = pow(max(dot(halfwayDir, norm), 0.0), shininess);
    return spec * specularStrength;
}

vec3 calcLight(vec3 viewDir, vec3 norm, vec3 col) {
    float diff = diffuse(norm, lightCam.dir_far.xyz);

    float spec = specular(norm, viewDir, lightCam.dir_far.xyz, 0.0, 32);
    vec3 light = clamp((spec + diff), 0.0, 1.0) * lightColor.xyz + ambientColor.xyz;

    vec3 color = light * col;
    
    return color;

}

float calcShadow(vec3 pos, out vec3 projCoords) {
    vec2 poissonDisk[4] = vec2[](
        vec2( -0.94201624, -0.39906216 ),
        vec2( 0.94558609, -0.76890725 ),
        vec2( -0.094184101, -0.92938870 ),
        vec2( 0.34495938, 0.29387760 )
    );
    vec4 posLightSpace = lightCam.vp * vec4(pos, 1.0);

    projCoords = posLightSpace.xyz / posLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5; 

    //if(projCoords.x >= 1.0 || projCoords.x <= 0.0 || projCoords.y >= 1.0 ||projCoords.y <= 0.0)
    //    return 1.0;

    float bias = 0.001;
    float visibility = 1.0;
    for (int i=0;i<4;i++){
        vec2 newUv = projCoords.xy + poissonDisk[i]/1400.0/3.0;
        if(newUv.x < 1.0 && newUv.y < 1.0 && newUv.x > 0.0 && newUv.y > 0.0)
            visibility -= 0.2 * (1.0 - texture(shadowMap, vec3(newUv, projCoords.z - bias)));
            //visibility -= 0.2 * float(projCoords.z - bias > texture(shadowMap, newUv).r);
    }
    return visibility;
}

void main()
{

    vec3 normalTex = texture(material.normalMap, uv).xyz * 2.0 - 1.0;

    float minSqrDist = 10000.0f;

    for(int i = 0; i < obstacleCount; i++) {
        vec4 obstacle = obstacles[i];
        vec3 dif = fragPos - obstacle.xyz;
        minSqrDist = min(minSqrDist, dot(dif, dif) - (obstacle.w * obstacle.w));
    }

    minSqrDist = clamp(minSqrDist, 0, 1);

    float blend = pow(minSqrDist, 0.5);

    normalTex.z /= material.normalMapStrength;

    vec3 norm = normalize(TBN * normalTex);

    vec3 grassCol = texture(material.grassTex, uv).xyz * material.grassColorMultiplier;
    vec3 dirtCol = texture(material.dirtTex, uv).xyz * material.dirtColorMultiplier;
    vec3 col = mix(grassCol, dirtCol, blend);
    vec3 projCoords;
    float shadow = calcShadow(fragPos, projCoords);
    
    FragColor = vec4(calcLight(normalize(cam.pos_near.xyz - fragPos), norm, col) * calcShadow(fragPos, projCoords), 1.0);
    FragColor = vec4(vec3(1.0) * calcShadow(fragPos, projCoords), 1.0);
    //FragColor = vec4(vec3(shadow) * vec3(projCoords.xy, 0), 1);
    //FragColor = vec4(norm, 1.0);
} 