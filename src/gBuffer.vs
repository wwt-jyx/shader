#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec3 aTangent;
layout (location = 3) in vec2 aTexCoord_0;
layout (location = 4) in vec2 aTexCoord_1;

out vec2 TexCoord_0;
out vec3 ViewPos;
out vec3 Normal;

uniform mat4 model;
layout (std140) uniform Matrices
{
    mat4 projection;
    mat4 view;
};

void main()
{
    //世界空间中的观察位置
    ViewPos = vec3(view * model * vec4(aPos, 1.0));
    gl_Position =  projection * view  * model * vec4(aPos, 1.0);

    TexCoord_0 = vec2(aTexCoord_0.x,1-aTexCoord_0.y);

    //法向量也转换为观察空间坐标
    Normal =  mat3(transpose(inverse(view  * model))) *  aNormal;
}