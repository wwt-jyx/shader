#version 330 core
out vec4 FragColor;

in vec4 FragPosLightSpace;
in vec2 TexCoord;
uniform sampler2D shadowMap;

uniform float near_plane;
uniform float far_plane;
float ShadowCalculation(vec4 fragPosLightSpace)
{
    // 执行透视除法
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    // 变换到[0,1]的范围
    projCoords = projCoords * 0.5 + 0.5;
    // 取得最近点的深度(使用[0,1]范围下的fragPosLight当坐标)
    float closestDepth = texture(shadowMap, projCoords.xy).r;
    // 取得当前片段在光源视角下的深度
    float currentDepth = projCoords.z;
    // 检查当前片段是否在阴影中
    float bias = 0.00005;
//     float shadow = currentDepth - bias> closestDepth  ? 1.0 : 0.0;

    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
    for(int x = -1; x <= 1; ++x)
    {
        for(int y = -1; y <= 1; ++y)
        {
            float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r;
            shadow += currentDepth - bias > pcfDepth ? 0.4 : 0.0;
        }
    }
    shadow /= 9.0;
    if(projCoords.z > 1.0)
            shadow = 0.0;


    return shadow;
}

float LinearizeDepth(float depth)
{
    float z = depth * 2.0 - 1.0; // Back to NDC
    return (2.0 * near_plane * far_plane) / (far_plane + near_plane - z * (far_plane - near_plane));
}

void main()
{
    float shadow = ShadowCalculation(FragPosLightSpace);
    vec3 color = vec3(0.8) * (1-shadow);
//     vec3 color = vec3(texture(shadowMap, TexCoord));

//         float depthValue = texture(shadowMap, TexCoord).r;
//         vec4 color = vec4(vec3(LinearizeDepth(depthValue) / far_plane), 1.0); // perspective
//         vec4 color = vec4(0.4);  // orthographic
//         FragColor = color;

    FragColor = vec4(color,1.0);
}