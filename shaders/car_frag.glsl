#version 450 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D texture_diffuse1;
uniform uint u_materialID;

uniform bool u_braking;

void main()
{
    vec4 texColor = texture(texture_diffuse1, TexCoords);

    if (texColor.a < 0.1)
    discard;

    if (u_materialID == 9) {
        if (u_braking)
        texColor *= vec4(0.7f, 0.2f, 0.2f, 1.0f);
        else
        texColor *= vec4(0.3f, 0.3f, 0.3f, 1.0f);
    }

    FragColor = texColor;
}


