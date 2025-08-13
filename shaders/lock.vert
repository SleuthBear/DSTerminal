#version 330 core
layout (location = 0) in vec4 aPos;
uniform mat4 projection;

void main()
{
    // simple 2D position
    gl_Position = projection * vec4(aPos.xy, 0.0, 1.0);
}

