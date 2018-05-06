#version 330 core

in vec2 fragTexCoord;
in vec3 fragColor;

out vec4 finalColor;

uniform sampler2D texture0;
uniform sampler2D colorbuffer;
uniform sampler2D normalbuffer;
uniform sampler2D positionbuffer;
uniform sampler2D depthbuffer;
uniform sampler2D noisebuffer;

struct light {
    vec3 position;
    vec3 color;
    
    float linear;
    float quadratic;
};
const int num_lights = 32;
uniform light lights[num_lights];
uniform vec3 viewpos;

uniform mat4 projection;

uniform vec3 samples[64];
const vec2 noisescale = vec2(1280.f/4.f, 720.f/4.f);

const float factor = 1.0f / 64.0f;

vec3 calc_lighting()
{
    vec3 Normal = texture(normalbuffer, fragTexCoord).rgb;
    vec3 FragPos = texture(positionbuffer, fragTexCoord).rgb;
    vec3 Diffuse = texture(colorbuffer, fragTexCoord).rgb;
    float Specular = texture(colorbuffer, fragTexCoord).a;
    
    vec3 lighting = Diffuse * 0.1;
    vec3 viewdir = normalize(viewpos - FragPos);
    for (int i = 0; i < num_lights; i++) {
        vec3 lightdir = normalize(lights[i].position - FragPos);
        vec3 diffuse = max(dot(Normal, lightdir), 0.0) * Diffuse * lights[i].color;
        
        vec3 halfwaydir = normalize(lightdir + viewdir);
        float spec = pow(max(dot(Normal, halfwaydir), 0.0), 16.0);
        vec3 specular = lights[i].color * spec * Specular;
        
        float distance = length(lights[i].position - FragPos);
        float attenuation = 1.0 / (1.0 + lights[i].linear * distance + lights[i].quadratic * distance * distance);
        diffuse *= attenuation;
        specular *= attenuation;
        lighting += diffuse + specular;
    }
    
    return lighting;
    
}

vec2 saturate(vec2 x)
{
    vec2 t;
    float a = x.x;
    float b = x.y;
    t.x = max(0, min(1, a));
    t.y = max(0, min(1, b));
    return t;
  // return max(vec2(0), min(vec2(1), x));
}

float saturate(float x)
{
    return max(0, min(1, x));
}

vec3 depthnormal(float depth)
{
    const vec2 offset1 = vec2(0.0f,0.001f);
    const vec2 offset2 = vec2(0.001f,0.0f);
    
    float depth1 = texture(depthbuffer, fragTexCoord+offset1).r;
    float depth2 = texture(depthbuffer, fragTexCoord+offset2).r;
    
    vec3 p1 = vec3(offset1, depth1 - depth);
    vec3 p2 = vec3(offset2, depth2 - depth);
    
    vec3 normal = cross(p1,p2);
    normal.z = -normal.z;
    
    return normalize(normal);
}

vec3 calc_ssao()
{
    vec3 Normal = texture(normalbuffer, fragTexCoord).rgb;
    vec3 FragPos = texture(positionbuffer, fragTexCoord).rgb;
    vec3 Diffuse = texture(colorbuffer, fragTexCoord).rgb;
    float Specular = texture(colorbuffer, fragTexCoord).a;
    
    vec3 randomVec = normalize(texture(noisebuffer, fragTexCoord*noisescale).rgb).xyz;
    
    // vec3 tangent = normalize(randomVec - Normal * dot(randomVec, Normal)).xyz;
    // vec3 bitangent = cross(Normal, tangent).xyz;
    // mat3 TBN = mat3(tangent, bitangent, Normal);
    
    // float occlusion = 0.0;
    // for (int i = 0; i < 64; i++)
    // {
    //     vec3 samp = TBN * samples[i];
    //     samp = FragPos + samp * 0.5;
        
    //     vec4 offset = vec4(samp, 1.0);
    //     offset = projection * offset;
    //     offset.xyz /= offset.w;
    //     offset.xyz = offset.xyz * 0.5 + 0.5;
    //     offset.xy = vec2(offset.x, 1.0-offset.y);
        
    //     float sampleDepth = texture(positionbuffer, offset.xy).z;
        
    //     float rangeCheck = smoothstep(0.0, 1.0, 0.5 / abs(FragPos.z - sampleDepth));
    //     occlusion += (sampleDepth >= samp.z + 0.025 ? 1.0 : 0.0) * rangeCheck;
    // }
    
    // occlusion = 1.0f - (occlusion * factor);
    float depth = texture(depthbuffer, fragTexCoord).r;
    float f = 10.f;
    float n = 0.01f;
    // depth = 2.f*n/(f+n-depth*(f-n));
    Normal = depthnormal(depth);
    // return Normal;
    
    float radius_depth = 0.1f/depth;
    float occlusion = 0.f;
    for (int i = 0; i < 64; i++)
    {
        vec3 ray = radius_depth * reflect(samples[i], randomVec);
        vec3 hemi_ray = vec3(fragTexCoord, depth) + sign(dot(ray,Normal))*ray;
        float occ_depth = texture(depthbuffer, saturate(hemi_ray.xy)).r;
        float difference = depth - occ_depth;
        occlusion += step(0.000001f, difference) * (1.f-smoothstep(0.000001f, 0.0075f, difference));
    }
    float ao = 1.f - 1.f * occlusion * factor;
    occlusion = saturate(ao + 0.2f);
    
    return vec3(occlusion);
    
}

void main()
{
    
    vec3 color = vec3(0);
    color = calc_lighting();
    color *= calc_ssao();
    // color = texture(normalbuffer, fragTexCoord).rgb;
    
    finalColor = vec4(color, 1);//*texture(depthbuffer, fragTexCoord);
    
    if (fragTexCoord.x < 0.05 && fragTexCoord.y < 0.1)
        finalColor = vec4(viewpos, 1);
}
