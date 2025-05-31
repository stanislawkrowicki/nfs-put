#version 450 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D texture_diffuse1;
uniform sampler2D texture_basecolor1;
uniform sampler2D texture_normals1;

uniform bool u_hasBaseColorTexture;
uniform vec4 u_baseColor;
uniform uint u_materialID;

uniform bool u_braking;

void main()
{
    vec4 texColor = u_hasBaseColorTexture
    ? texture(texture_basecolor1, TexCoords)
    : u_baseColor;

    if (texColor.a <= 0.01)
    discard;

    if (u_materialID == 9) {
        if (u_braking)
        texColor *= vec4(0.7f, 0.2f, 0.2f, 1.0f);
        else
        texColor *= vec4(0.3f, 0.3f, 0.3f, 1.0f);
    }

    FragColor = texColor;
}


