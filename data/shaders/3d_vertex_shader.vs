#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;

out vec3 vertexColor;
out vec3 normal;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

void main()
{
	gl_Position = projection * view * model * vec4(aPos, 1.0);
	vertexColor = aColor;
	normal = vec3(0, 1, 0);
}