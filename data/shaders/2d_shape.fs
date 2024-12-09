#version 330 core
out vec4 FragColor;

uniform vec4 shape_color;

void main()
{
    FragColor = shape_color;
}