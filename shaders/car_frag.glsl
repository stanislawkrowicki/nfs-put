#version 450 core

in vec2 TexCoords;
in vec3 Normal;
in vec3 WorldPos;

out vec4 FragColor;

uniform sampler2D texture_basecolor1;

uniform bool u_hasBaseColorTexture;
uniform vec4 u_baseColor;
uniform uint u_materialID;

uniform vec4 u_bodyColor;

uniform bool u_braking;

uniform vec3 u_lightColor;
uniform vec3 u_lightPos;
uniform float u_lightIntensity;
uniform vec3 u_camPos;

void main()
{
    vec4 texColor = u_hasBaseColorTexture
    ? texture(texture_basecolor1, TexCoords)
    : u_baseColor;

    if (texColor.a < 0.1)
    discard;

/* Override car body color */
    if (u_materialID == 5)
    texColor = u_bodyColor;

    vec3 objColor = texColor.rgb;
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(u_lightPos - WorldPos);
    vec3 viewDir = normalize(u_camPos - WorldPos);
    vec3 lightColor = u_lightColor * u_lightIntensity;

    float ambientStrength = 0.45;
    vec3 ambientLight = ambientStrength * lightColor;

    float diffuseStrength = max(dot(norm, lightDir), 0.0);
    vec3 diffuseLight = diffuseStrength * lightColor;

    vec3 reflectDir = reflect(-lightDir, norm);
    float specularStrength = 0.5;
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 64.0);
    vec3 specularLight = specularStrength * spec * lightColor;

    float distance = length(u_lightPos - WorldPos);
    float attenuation = 1.0 / (1.0 + 0.09 * distance + 0.032 * distance * distance);
    float ao = pow(clamp(dot(norm, lightDir), 0.0, 1.0), 2.0);
    vec3 result = objColor * (ambientLight + ao * diffuseLight + specularLight);

    float fresnel = pow(1.0 - max(dot(viewDir, norm), 0.0), 5.0);
    result += fresnel * vec3(0.1, 0.1, 0.15);

    if (u_materialID == 7) {
        if (u_braking)
        result *= vec3(0.7, 0.2, 0.2);
        else
        result *= vec3(0.3, 0.3, 0.3);
    }

    FragColor = vec4(pow(result, vec3(1.0 / 1.2)), 1.0);
}
