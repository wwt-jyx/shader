#version 330 core
layout (location = 0) out vec4 gPositionDepth;
layout (location = 1) out vec4 gNormal;
layout (location = 2) out vec4 gAlbedoSpec;

in vec2 TexCoord_0;
in vec3 ViewPos;
in vec3 Normal;

struct Material {
    sampler2D baseColorTexture;
    bool hasNormalTex;
    sampler2D normalTexture;
    sampler2D metallicRoughnessTexture; //(B)Metallic (G)Roughness

    int alphaMode;
};
uniform Material material;
uniform float near_plane;
uniform float far_plane;

float LinearizeDepth(float depth)
{
    float z = depth * 2.0 - 1.0; // Back to NDC
    return (2.0 * near_plane * far_plane) / (far_plane + near_plane - z * (far_plane - near_plane));
}


void main()
{
    // Store the fragment position vector in the first gbuffer texture
    gPositionDepth.xyz = ViewPos;
    gPositionDepth.a = 1;
//     gl_FragDepth = LinearizeDepth(gl_FragCoord.z)/ far_plane;
    gl_FragDepth = gl_FragCoord.z;

    // Also store the per-fragment normals into the gbuffer
    gNormal = vec4(normalize(Normal),1.0);
    // And the diffuse per-fragment color
    gAlbedoSpec.rgb = texture(material.baseColorTexture, TexCoord_0).rgb;
    // Store specular intensity in gAlbedoSpec's alpha component
//     gAlbedoSpec.a = texture(texture_specular1, TexCoord_0).r;
}