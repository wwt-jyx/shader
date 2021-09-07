#version 330 core

layout (location = 0) out vec4 FragColor;
layout (location = 1) out vec4 BrightColor;

in vec2 TexCoord_0;
in vec3 Normal;
in vec3 FragPos;

struct Material {
    //ambient材质向量定义了在环境光照下这个物体反射得是什么颜色，通常这是和物体颜色相同的颜色。
    //diffuse材质向量定义了在漫反射光照下物体的颜色。（漫反射颜色也要设置为我们需要的物体颜色。
    //specular材质向量设置的是镜面光照对物体的颜色影响（或者甚至可能反射一个物体特定的镜面高光颜色）。
    //shininess影响镜面高光的散射/半径。
    sampler2D baseColorTexture;
    sampler2D normalTexture;
    vec3 specular;
    float shininess;
};

uniform Material material;

struct DirLight {
    vec3 direction;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};
uniform DirLight dirLight;
vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir);

struct PointLight {
    vec3 position;

    float constant; //常数项
    float linear;   //一次项
    float quadratic;//二次项

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};
#define NR_POINT_LIGHTS 3
uniform PointLight pointLights[NR_POINT_LIGHTS];
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir);

struct SpotLight {
    vec3  position;

    vec3  direction;
    float cutOff;          //内圆锥的余弦
    float outerCutOff;     //外圆锥的余弦

    float constant; //常数项
    float linear;   //一次项
    float quadratic;//二次项

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};
uniform SpotLight spotLight;
vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir);

//半球光（HemisphereLight）的颜色是从天空到地面两个颜色之间的渐变，与物体材质的颜色作叠加后得到最终的颜色效果。
//一个点受到的光照颜色是由所在平面的朝向（法向量）决定的 —— 面向正上方就受到天空的光照颜色，
//面向正下方就受到地面的光照颜色，其他角度则是两个颜色渐变区间的颜色。
struct HemisphereLight {
    vec3 skyColor;     //"天空"的颜色
    vec3 groundColor;  //"地面反射"的颜色
    float skyIntensity;   //"天空"的强度
    float groundIntensity;   //"地面反射"强度

    vec3  direction;
};
uniform HemisphereLight hemisphereLight;
vec3 CalcHemisphereLight(HemisphereLight light, vec3 normal, vec3 fragPos, vec3 viewDir);


//uniform sampler2D baseColorTexture;
uniform sampler2D metallicRoughnessTexture;


uniform samplerCube skybox;

uniform int alphaMode;
uniform vec3 viewPos;
uniform bool blinn;
uniform bool hasNormalTex;
uniform mat4 model;

in VS_OUT {
    mat3 TBN;
} fs_in;
void main()
{
   vec3 baseColor = vec3(texture(material.baseColorTexture, TexCoord_0));
   if(alphaMode==2)
        discard;
    // 属性
//     vec3 norm = normalize(Normal);
//     // 从法线贴图范围[0,1]获取法线
//     vec3 norm = texture(material.normalTexture, TexCoord_0).rgb;
//     // 将法线向量转换为范围[-1,1]
//     norm = normalize(norm * 2.0 - 1.0);
//     norm  = mat3(transpose(inverse(model))) * norm ;
//     norm = normalize(norm);
    vec3 norm,viewDir;
    if(hasNormalTex)
    {
        norm = texture(material.normalTexture, TexCoord_0).rgb;
        norm = normalize(norm * 2.0 - 1.0);
        viewDir = fs_in.TBN * normalize(viewPos - FragPos);
    }
    else
    {
        norm = normalize(Normal);
        viewDir = normalize(viewPos - FragPos);
    }


    vec3 result = vec3(0.0,0.0,0.0);
    // 第0阶段：环境光
    result += vec3(0.2,0.2,0.2)*vec3(texture(material.baseColorTexture, TexCoord_0));
    // 第一阶段：定向光照
//     result += CalcDirLight(dirLight, norm, viewDir);
    // 第二阶段：点光源
    for(int i = 0; i < NR_POINT_LIGHTS; i++)
        result += CalcPointLight(pointLights[i], norm, FragPos, viewDir);
    // 第三阶段：聚光
    result += CalcSpotLight(spotLight, norm, FragPos, viewDir);
    // 第四阶段：半球光
//     result += CalcHemisphereLight(hemisphereLight, norm, FragPos, viewDir);

//     //反射
//     vec3 I = normalize(FragPos - viewPos);
//     vec3 R = reflect(I, norm);
//     vec4 reflectColor = vec4(texture(skybox, R).rgb, 1.0);
//
//     //折射
//     float ratio = 1.00 / 1.52;
//     vec3 Rr = refract(I, norm, ratio);
//     vec4 refractColor = vec4(texture(skybox, Rr).rgb, 1.0);

    FragColor = vec4(result,1.0);

    // Check whether fragment output is higher than threshold, if so output as brightness color
    float brightness = dot(FragColor.rgb, vec3(0.2126, 0.7152, 0.0722));
    if(brightness > 1.0)
        BrightColor = vec4(FragColor.rgb, 1.0);
    else
        BrightColor = vec4(0.0,0.0,0.0, 1.0);
}

vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir)
{
    vec3 lightDir = normalize(-light.direction);
    if(hasNormalTex)
        lightDir = fs_in.TBN * lightDir;
    // 漫反射着色
    float diff = max(dot(normal, lightDir), 0.0);
    // 镜面光着色
    float spec= 0.0;;
    if(blinn)
    {
        vec3 halfwayDir = normalize(lightDir + viewDir);
        spec = pow(max(dot(normal, halfwayDir), 0.0), 16.0);
    }
    else
    {
        vec3 reflectDir = reflect(-lightDir, normal);
        spec = pow(max(dot(viewDir, reflectDir), 0.0), 8.0);
    }
    // 合并结果
    vec3 ambient  = light.ambient  *  vec3(texture(material.baseColorTexture, TexCoord_0));
    vec3 diffuse  = light.diffuse  * diff *  vec3(texture(material.baseColorTexture, TexCoord_0));
    vec3 specular = light.specular * spec * material.specular;
    return (ambient + diffuse + specular);
}

vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir)
{
    vec3 lightDir = normalize(light.position - fragPos);
    if(hasNormalTex)
        lightDir = fs_in.TBN * lightDir;
    // 漫反射着色
    float diff = max(dot(normal, lightDir), 0.0);
    // 镜面光着色
    float spec = 0.0;;
    if(blinn)
    {
        vec3 halfwayDir = normalize(lightDir + viewDir);
        spec = pow(max(dot(normal, halfwayDir), 0.0), 16.0);
    }
    else
    {
        vec3 reflectDir = reflect(-lightDir, normal);
        spec = pow(max(dot(viewDir, reflectDir), 0.0), 8.0);
    }
    // 衰减
    float distance    = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance +
                 light.quadratic * (distance * distance));
    // 合并结果
    vec3 ambient  = light.ambient  *  vec3(texture(material.baseColorTexture, TexCoord_0));
    vec3 diffuse  = light.diffuse  * diff *  vec3(texture(material.baseColorTexture, TexCoord_0));
    vec3 specular = light.specular * spec * material.specular;
//     ambient  *= attenuation;
//     diffuse  *= attenuation;
//     specular *= attenuation;
    return (ambient + diffuse + specular);
}

vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir)
{
    vec3 lightDir = normalize(light.position - fragPos);
    if(hasNormalTex)
        lightDir = fs_in.TBN * lightDir;
    // spotlight (soft edges)
    float theta     = dot(lightDir, normalize(-light.direction));
    float epsilon   = light.cutOff - light.outerCutOff;
    float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);

    // 执行光照计算
    // 漫反射着色
    float diff = max(dot(normal, lightDir), 0.0);
    // 镜面光着色
    float spec = 0.0;;
    if(blinn)
    {
        vec3 halfwayDir = normalize(lightDir + viewDir);
        spec = pow(max(dot(normal, halfwayDir), 0.0), 16.0);
    }
    else
    {
        vec3 reflectDir = reflect(-lightDir, normal);
        spec = pow(max(dot(viewDir, reflectDir), 0.0), 8.0);
    }
    // 衰减
    float distance    = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance +
                   light.quadratic * (distance * distance));
    // 合并结果
    vec3 ambient  = light.ambient  *  vec3(texture(material.baseColorTexture, TexCoord_0));
    vec3 diffuse  = light.diffuse  * diff *  vec3(texture(material.baseColorTexture, TexCoord_0));
    vec3 specular = light.specular * spec * material.specular;

    diffuse  *= intensity;
    specular *= intensity;

    ambient  *= attenuation;
    diffuse  *= attenuation;
    specular *= attenuation;

    return (ambient + diffuse + specular);

}


vec3 CalcHemisphereLight(HemisphereLight light, vec3 normal, vec3 fragPos, vec3 viewDir)
{
    vec3 lightDir = normalize(-light.direction);
    if(hasNormalTex)
        lightDir = fs_in.TBN * lightDir;
    float costheta = max(dot(normal,  lightDir), 0.0);
    float a = 0.5 + 0.5 * costheta;
    vec3 color = mix(light.skyColor*light.skyIntensity,light.groundColor*light.groundIntensity,a);
    return color * vec3(texture(material.baseColorTexture, TexCoord_0));

}