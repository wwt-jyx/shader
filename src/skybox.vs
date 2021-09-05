#version 330 core
layout (location = 0) in vec3 aPos;

out vec3 TexCoords;

layout (std140) uniform Matrices
{
    mat4 projection;
    mat4 view;
};

void main()
{
    TexCoords = vec3(aPos.x , -1 * aPos.y , aPos.z);
    vec4 pos = projection * view * vec4(aPos, 1.0);
    gl_Position = pos.xyww;
}