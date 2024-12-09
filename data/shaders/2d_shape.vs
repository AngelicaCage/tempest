#version 330 core
layout (location = 0) in vec3 aPos;

uniform vec2 shape_scale;
uniform vec2 shape_offset;

void main()
{
	gl_Position = vec4(aPos.x * shape_scale.x + shape_offset.x,
                       aPos.y * shape_scale.y + shape_offset.y,
                       aPos.z, 1);
}