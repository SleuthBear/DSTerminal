#version 330 core
in vec2 TexCoords;
out vec4 color;

uniform vec3 spriteColor;
uniform sampler2D sprite;

void main()
{
    color = texture(sprite, TexCoords) * vec4(spriteColor, 1.0);
}

