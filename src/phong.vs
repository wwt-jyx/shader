#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec3 aTangent;
layout (location = 3) in vec2 aTexCoord_0;
layout (location = 4) in vec2 aTexCoord_1;

out vec2 TexCoord_0;
out vec3 Normal;
out vec3 FragPos;

uniform mat4 model;
layout (std140) uniform Matrices
{
    mat4 projection;
    mat4 view;
};

out VS_OUT {
    mat3 TBN;
} vs_out;

void main()
{

    //法向量也转换为世界空间坐标
    Normal = mat3(transpose(inverse(model))) * aNormal;

    //世界空间中的顶点位置
    FragPos = vec3(model * vec4(aPos, 1.0));

    //纹理坐标转置
    TexCoord_0 = vec2(aTexCoord_0.x,1-aTexCoord_0.y);

    //
    gl_Position = projection * view * model * vec4(aPos, 1.0);

    //TBN
    vec3 T = normalize(mat3(transpose(inverse(model))) * aTangent);
    vec3 N = normalize(mat3(transpose(inverse(model))) * aNormal);
    vec3 B = normalize(cross(T, N));
    vs_out.TBN = transpose(mat3(T, B, N));
}