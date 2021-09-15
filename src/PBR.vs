#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec3 aTangent;
layout (location = 3) in vec2 aTexCoord_0;
layout (location = 4) in vec2 aTexCoord_1;

out vec2 TexCoord_0;
out vec3 WorldPos;
out vec3 Normal;
out vec3 Tangent;


layout (std140) uniform Matrices
{
    mat4 projection;
    mat4 view;
};
uniform mat4 model;

void main()
{

    TexCoord_0 = vec2(aTexCoord_0.x,1-aTexCoord_0.y);
    //世界空间中的顶点位置
    WorldPos = vec3(model * vec4(aPos, 1.0));
    //法向量也转换为世界空间坐标
    Normal =  mat3(transpose(inverse(model))) *  aNormal;
    Tangent = mat3(transpose(inverse(model))) * aTangent;

    gl_Position =  projection * view * vec4(WorldPos, 1.0);

    //TBN
    //vec3 T = normalize(mat3(transpose(inverse(model))) * aTangent);
    //vec3 N = normalize(mat3(transpose(inverse(model))) * aNormal);
    //vec3 B = normalize(cross(T, N));
    //TBN = transpose(mat3(T, B, N));
}