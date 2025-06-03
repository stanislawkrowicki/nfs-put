#version 450 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D screenTexture;
uniform float blurAmount;
uniform vec3 carScreenPos;

void main() {
    float offset = 1.0 / 300.0; // blur spread

    vec2 pos2D = carScreenPos.xy;
    float dist = distance(TexCoords, pos2D);
    float localBlur = clamp(dist * blurAmount * 5.0, 0.0, blurAmount);


    vec3 result = texture(screenTexture, TexCoords).rgb * 0.2;
    result += texture(screenTexture, TexCoords + vec2(offset, 0.0) * localBlur).rgb * 0.2;
    result += texture(screenTexture, TexCoords - vec2(offset, 0.0) * localBlur).rgb * 0.2;
    result += texture(screenTexture, TexCoords + vec2(0.0, offset) * localBlur).rgb * 0.2;
    result += texture(screenTexture, TexCoords - vec2(0.0, offset) * localBlur).rgb * 0.2;

    FragColor = vec4(result, 1.0);
}

