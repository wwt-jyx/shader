#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec3 aTangent;
layout (location = 3) in vec2 aTexCoord_0;
layout (location = 4) in vec2 aTexCoord_1;

out vec2 TexCoord_0;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    TexCoord_0 = aTexCoord_0;
    gl_Position = projection * view * model * vec4(aPos, 1.0);
}