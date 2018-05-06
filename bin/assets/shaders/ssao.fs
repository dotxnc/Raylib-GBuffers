#version 330

out vec4 finalColor;

in vec2 fragTexCoord;

uniform sampler2D texture0;
uniform sampler2D normalbuffer;
uniform sampler2D depthbuffer;
uniform sampler2D noisebuffer;
uniform vec3 samples[64];

const vec2 noisescale = vec2(1280.f/4.f, 720.f/4.f);
const float factor = 1.f / 64.f;

vec2 saturate(vec2 x)
{
    vec2 t;
    float a = x.x;
    float b = x.y;
    t.x = max(0, min(1, a));
    t.y = max(0, min(1, b));
    return t;
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
    
    vec3 randomVec = normalize(texture(noisebuffer, fragTexCoord*noisescale).rgb).xyz;
    
    float depth = texture(depthbuffer, fragTexCoord).r;
    // Normal = depthnormal(depth);
    
    float radius_depth = 0.5f/depth;
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
    vec3 color = calc_ssao();
    finalColor = vec4(color, 1.f);
    // finalColor = texture(normalbuffer, fragTexCoord);
}
