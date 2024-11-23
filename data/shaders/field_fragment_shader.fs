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

	vec3 norm = normalize(normal);
	vec3 lightDir = normalize(-sunLightDirection);
	float diff = max(dot(norm, lightDir), 0.0);
	float sunLightIntensity = sunLightStrength * diff;

/*
	if(sunLightIntensity > 0.5)
		sunLightIntensity = 1.0;
	else
		sunLightIntensity = 0.0;
*/
	vec3 diffuse = sunLightIntensity * sunLightColor;
	FragColor = vec4(vertexColor * (ambient + diffuse), 0.7f);
}
