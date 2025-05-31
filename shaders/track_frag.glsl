#version 450 core

#define MAX_BRAKE_LIGHTS 8
#define brakeLightCutoff - 0.86 // cosine of cutoff angle (210deg atm)

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

uniform int u_brakeLightCount;
/* we can use mat2, would be cleaner */
uniform vec3 u_brakeLightPositions[MAX_BRAKE_LIGHTS];
uniform vec3 u_brakeLightDirections[MAX_BRAKE_LIGHTS];


void main() {
    vec4 texColor = texture(texture_diffuse1, TexCoords);

    if (texColor.a < 0.01)
    discard;

    vec3 objColor = texColor.rgb;
    vec3 lightColor = u_lightColor * u_lightIntensity;

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

    // Brake lights
    for (int i = 0; i < u_brakeLightCount; ++i) {
        vec3 brakePos = u_brakeLightPositions[i];
        vec3 brakeDir = normalize(u_brakeLightDirections[i]);
        vec3 fragToLight = normalize(brakePos - WorldPos);

        float theta = dot(-fragToLight, brakeDir);
        if (theta < brakeLightCutoff) continue;

        float distance = length(brakePos - WorldPos);

        float attenuation = 1.0 / (1.0 + 0.5 * distance + 1.0 * distance * distance);

        float influence = clamp(theta, 0.0, 1.0);
        influence = pow(influence, 6.0); // tighten the beam

        vec3 brakeColor = vec3(1.0, 0.0, 0.0);
        vec3 brakeLight = brakeColor * attenuation * influence;

        result += brakeLight * 1.3;
    }

    result += fresnel * vec3(0.1, 0.1, 0.15); // delikatny b?ysk

    FragColor = vec4(pow(result, vec3(1.0 / 1.2)), 1.0);
}


