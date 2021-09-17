#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 3) in vec2 aTexCoord;
layout (std140) uniform Matrices
{
    mat4 projection;
    mat4 view;
};
out vec4 FragPosLightSpace;
out vec2 TexCoord;

uniform mat4 model;
uniform mat4 lightSpaceMatrix;

void main()
{
    gl_Position = projection * view * vec4(aPos, 1.0);
    FragPosLightSpace = lightSpaceMatrix * vec4(aPos, 1.0);
    TexCoord = aTexCoord;
}