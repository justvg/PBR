#version 330 core
out vec4 FragColor;

in vec3 LocalPos;

uniform samplerCube Skybox;

void main()
{
	vec3 Color = texture(Skybox, LocalPos).rgb;

	Color = Color / (Color + vec3(1.0));
	Color = sqrt(Color);

	FragColor = vec4(Color, 1.0);
}