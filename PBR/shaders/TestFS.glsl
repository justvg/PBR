#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D Texture;

void main()
{
	vec3 Color = texture(Texture, TexCoords).rgb;
	FragColor = vec4(Color, 1.0);
}