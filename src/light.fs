#version 330 core

layout (location = 0) out vec4 FragColor;
layout (location = 1) out vec4 BrightColor;

void main()
{
    FragColor = vec4(1.0); // 将向量的四个分量全部设置为1.0
    BrightColor = FragColor;
}