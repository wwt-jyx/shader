#version 330 core
out vec4 FragColor;

in vec2 TexCoord_0;

uniform sampler2D baseColorTexture;

void main()
{
    FragColor = texture(baseColorTexture, TexCoord_0);
}