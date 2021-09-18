#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D screenTexture;
uniform sampler2D bloomBlur;
uniform float near_plane;
uniform float far_plane;

const float offset = 1.0 / 300.0;
vec2 offsets[9] = vec2[](
        vec2(-offset,  offset), // 左上
        vec2( 0.0f,    offset), // 正上
        vec2( offset,  offset), // 右上
        vec2(-offset,  0.0f),   // 左
        vec2( 0.0f,    0.0f),   // 中
        vec2( offset,  0.0f),   // 右
        vec2(-offset, -offset), // 左下
        vec2( 0.0f,   -offset), // 正下
        vec2( offset, -offset)  // 右下
    );

float LinearizeDepth(float depth)
{
    float z = depth * 2.0 - 1.0; // Back to NDC
    return (2.0 * near_plane * far_plane) / (far_plane + near_plane - z * (far_plane - near_plane));
}


void main()
{
    vec3 hdrColor = texture(screenTexture, TexCoords).rgb;
//     vec3 bloomColor = texture(bloomBlur, TexCoords).rgb;
//     hdrColor += bloomColor; // additive blending
    // Reinhard色调映射
//     vec3 mapped = hdrColor / (hdrColor + vec3(1.0));
    // 曝光色调映射
//     float exposure=1.0;
//     vec3 mapped = vec3(1.0) - exp(-hdrColor * exposure);
    FragColor = vec4( hdrColor, 1.0);

//     float depthValue = texture(screenTexture, TexCoords).r;
//     vec4 color = vec4(vec3(LinearizeDepth(depthValue) / far_plane), 1.0); // perspective
//     vec4 color = vec4(vec3(depthValue), 1.0);  // orthographic
//     FragColor = color;


    //gamma校正
//     vec3 fragColor = vec3(texture(screenTexture, TexCoords));
//     float gamma = 1;
//     FragColor.rgb = pow(fragColor.rgb, vec3(1.0/gamma));



    //反相
//     FragColor = vec4(vec3(1.0 - texture(screenTexture, TexCoords)), 1.0);

    //灰度
//     FragColor = texture(screenTexture, TexCoords);
//     float average = 0.2126 * FragColor.r + 0.7152 * FragColor.g + 0.0722 * FragColor.b;
//     FragColor = vec4(average, average, average, 1.0);

    //锐化(Sharpen)核
//     float SharpenKernel[9] = float[](
//             -1, -1, -1,
//             -1,  9, -1,
//             -1, -1, -1
//         );
//     //模糊(Blur)
//     float BlurKernel[9] = float[](
//           1.0 / 16, 2.0 / 16, 1.0 / 16,
//           2.0 / 16, 4.0 / 16, 2.0 / 16,
//           1.0 / 16, 2.0 / 16, 1.0 / 16
//       );
//     //边缘检测(Edge-detection)
//     float EdgeKernel[9] = float[](
//             1,  1,  1,
//             1,  8,  1,
//             1,  1,  1
//       );
//     vec3 sampleTex[9];
//     for(int i = 0; i < 9; i++)
//     {
//         sampleTex[i] = vec3(texture(screenTexture, TexCoords.st + offsets[i]));
//     }
//     vec3 col = vec3(0.0);
//     for(int i = 0; i < 9; i++)
//         col += sampleTex[i] * EdgeKernel[i];
//
//     FragColor = vec4(col, 1.0);
}