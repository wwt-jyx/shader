#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D gPositionDepth;
uniform sampler2D gNormal;
uniform sampler2D texNoise;
uniform sampler2D shadowMap;

uniform vec3 samples[64];

// parameters (you'd probably want to use them as uniforms to more easily tweak the effect)
int kernelSize = 64;
float radius = 100.0;

// tile noise texture over screen based on screen dimensions divided by noise size
const vec2 noiseScale = vec2(800.0f/4.0f, 600.0f/4.0f);

uniform mat4 projection;
uniform float near_plane;
uniform float far_plane;
float LinearizeDepth(float depth)
{
    float z = depth * 2.0 - 1.0; // Back to NDC
    return (2.0 * near_plane * far_plane) / (far_plane + near_plane - z * (far_plane - near_plane));
}

void main()
{
    // Get input for SSAO algorithm
    vec3 fragPos = texture(gPositionDepth, TexCoords).xyz;
    vec3 normal = texture(gNormal, TexCoords).rgb;

    vec3 randomVec = texture(texNoise, TexCoords * noiseScale).xyz;
    // Create TBN change-of-basis matrix: from tangent-space to view-space 使用了一个随机向量来构造切线向量
    vec3 tangent = normalize(randomVec - normal * dot(randomVec, normal));
    vec3 bitangent = cross(normal, tangent);
    mat3 TBN = mat3(tangent, bitangent, normal);
    // Iterate over the sample kernel and calculate occlusion factor
    float occlusion = 0.0;
    float test;
    for(int i = 0; i < kernelSize; ++i)
    {
        // get sample position
        vec3 sample = TBN * samples[i]; // From tangent to view-space // 切线->观察空间
        sample = fragPos + sample * radius;

        // project sample position (to sample texture) (to get position on screen/texture)
        //变换sample到屏幕空间
        vec4 offset = vec4(sample, 1.0);
        offset = projection * offset; // from view to clip-space
        offset.xyz /= offset.w; // perspective divide
        offset.xyz = offset.xyz * 0.5 + 0.5; // transform to range 0.0 - 1.0

        // get sample depth
        float sampleDepth = texture(shadowMap, offset.xy).r; // Get depth value of kernel sample
        float origindepth = offset.z;

        float bias = 0.0000025;
        // range check & accumulate
        float rangeCheck = smoothstep(0.0, 1.0, radius / abs(fragPos.z - sampleDepth ));
        occlusion += (sampleDepth >= (origindepth + bias)? 1.0 : 0.0) * rangeCheck;
//          occlusion += (sampleDepth >= (origindepth + bias) ? 1.0 : 0.0);
         if(i==0) test=origindepth;
    }
    occlusion = 1.0 - (occlusion / kernelSize);

//     FragColor = vec4(occlusion,0.0,0.0,1.0);
    FragColor = vec4(1.0,0.0,0.0,1.0);
//     FragColor = vec4(samples[63],1.0);

//         float depthValue = texture(shadowMap, TexCoords).r;
//         float depthValue = test;
//         vec4 color = vec4(vec3(LinearizeDepth(depthValue) / far_plane), 1.0); // perspective
//         vec4 color = vec4(vec3(depthValue), 1.0);  // orthographic
//         FragColor = color;
}