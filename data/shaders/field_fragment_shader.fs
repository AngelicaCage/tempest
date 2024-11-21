#version 330 core

in vec3 vertexColor;
in vec3 normal;

out vec4 FragColor;

uniform vec3 ambientLightColor;
uniform float ambientLightStrength;
uniform vec3 sunLightColor;
uniform float sunLightStrength;
uniform vec3 sunLightDirection;

void main()
{
	vec3 ambient = ambientLightStrength * ambientLightColor;
	vec3 lightDir = normalize(-sunLightDirection);

	vec3 norm = normalize(normal);
	float diff = 1;//max(dot(norm, lightDir), 0.0);
	vec3 diffuse = sunLightStrength * diff * sunLightColor;

	FragColor = vec4(vertexColor * (ambient + diffuse), 1.0f);
}
