#version 330 core
out vec4 FragColor;

in vec2 TexCoord_0;

uniform sampler2D baseColorTexture;
uniform sampler2D metallicRoughnessTexture;
uniform sampler2D normalTexture;

void main()
{
    FragColor = texture(baseColorTexture, TexCoord_0);
}