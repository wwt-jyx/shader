#version 330 core
out vec4 FragColor;
in vec2 TexCoords;

uniform sampler2D nowColorMap;
uniform sampler2D oldColorMap;
uniform sampler2D alphaModeMap;
uniform int peel;
// ----------------------------------------------------------------------------
void main()
{
    vec3 Color;
    if(peel == 0)
    {
        Color = texture(nowColorMap, TexCoords).rgb;
    }

    else
    {
        if(texture(alphaModeMap, TexCoords).r == 1)
            Color = texture(oldColorMap, TexCoords).rgb * 0.5 + texture(nowColorMap, TexCoords).rgb * 0.5;
        else
            Color = texture(oldColorMap, TexCoords).rgb;

    }

    FragColor = vec4(Color, 1.0);
}