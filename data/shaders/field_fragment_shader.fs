#version 330 core

in vec3 vertexColor;

out vec4 FragColor;

uniform vec3 ambientLightColor;
uniform float ambientLightStrength;

void main()
{
	vec3 ambient = ambientLightStrength * ambientLightColor;

	FragColor = vec4(vertexColor * ambient, 1.0f);
}
