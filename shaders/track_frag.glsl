#version 450 core

in vec2 TexCoords;
in vec3 Normal;
in vec3 WorldPos;

out vec4 FragColor;

uniform vec3 u_objColor;
uniform vec3 u_lightColor;
uniform vec3 u_lightPos;
uniform vec3 u_camPos;
uniform sampler2D texture_diffuse1;
uniform float u_lightIntensity;

void main() {
    vec4 texColor = texture(texture_diffuse1, TexCoords);

    if (texColor.a < 0.01)
    discard;

    vec3 objColor = texColor.rgb;
    vec3 lightColor = u_lightColor;

    float ambientStrength = 0.45f;
    vec3 ambientLight = ambientStrength * lightColor;

    // Diffuse light
    vec3 lightDir = normalize(u_lightPos - WorldPos);
    vec3 norm = normalize(Normal);
    float diffuseStrength = max(dot(lightDir, norm), 0.0f);
    vec3 diffuseLight = diffuseStrength * lightColor;

    // Specular light
    vec3 viewDir = normalize(u_camPos - WorldPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float specularStrength = 0.5f;
    float spec = pow(max(dot(viewDir, reflectDir), 0.0f), 64);
    vec3 specularLight = specularStrength * spec * lightColor;

    // Combine all light factors together
    float distance = length(u_lightPos - WorldPos);
    float attenuation = 1.0 / (1.0 + 0.09 * distance + 0.032 * distance * distance);
    float ao = clamp(dot(norm, lightDir), 0.0, 1.0);
    ao = pow(ao, 2.0); // jeszcze ciemniejsze k?ty
    vec3 result = objColor * (ambientLight + ao * diffuseLight + specularLight);
    float fresnel = pow(1.0 - max(dot(viewDir, norm), 0.0), 5.0);
    result += fresnel * vec3(0.2, 0.2, 0.25); // delikatny b?ysk

    FragColor = vec4(pow(result, vec3(1.0 / 1.2)), 1.0);
}


