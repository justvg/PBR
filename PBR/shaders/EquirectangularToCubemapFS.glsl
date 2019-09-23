#version 330 core
out vec4 FragColor;

in vec3 LocalPos;

uniform sampler2D EquirectangularMap;

const vec2 InvAtan = vec2(0.1591, 0.3183);
vec2 SampleSphericalMap(vec3 V)
{
	vec2 UV = vec2(atan(V.z, V.x), asin(V.y));
	UV *= InvAtan;
	UV += 0.5;

	return (UV);
}

void main()
{
	vec2 UV = SampleSphericalMap(normalize(LocalPos));
	vec3 Color = texture(EquirectangularMap, UV).rgb;

	FragColor = vec4(Color, 1.0);
}