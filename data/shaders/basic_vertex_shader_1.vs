#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;

out vec3 vertexColor;
out vec3 vertexPos;

uniform vec3 offset;

void main()
{
	gl_Position = vec4(aPos + offset, 1.0);
	vertexColor = aColor;
	vertexPos = aPos + offset;
}