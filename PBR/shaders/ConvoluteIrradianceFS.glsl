#version 330 core
out vec4 FragColor;

in vec3 LocalPos;

uniform samplerCube EnvironmentMap;

const float PI = 3.14159265359;

void main()
{
	vec3 Normal = normalize(LocalPos);

	vec3 Irradiance = vec3(0.0);

	vec3 Up = vec3(0.0, 1.0, 0.0);
	vec3 Right = cross(Up, Normal);
	Up = cross(Normal, Right);

	float SampleDelta = 0.025;
	float SampleCount = 0;
	for(float Phi = 0.0; Phi < 2.0 * PI; Phi += SampleDelta)
	{
		for(float Theta = 0.0; Theta < 0.5 * PI; Theta += SampleDelta)
		{
			vec3 TangentSample = vec3(cos(Phi)*sin(Theta), sin(Phi)*sin(Theta), cos(Theta));

			vec3 SampleVec = TangentSample.x * Right + TangentSample.y * Up + TangentSample.z * Normal;

			Irradiance += texture(EnvironmentMap, SampleVec).rgb * cos(Theta) * sin((0.5f*PI) - Theta);
			SampleCount++;
		}
	}
	Irradiance = Irradiance / (SampleCount * PI);

	FragColor = vec4(Irradiance, 1.0);
}
