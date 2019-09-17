#version 330 core
out vec4 FragColor;

in vec2 TexCoords;
in vec3 FragPosWorld;
in vec3 Normal;

uniform vec3 Albedo;
uniform float Metallic;
uniform float Roughness;
uniform float AO;

uniform vec3 LightPositions[4];
uniform vec3 LightColors[4];

uniform vec3 CamPos;

const float PI = 3.14159265359;

vec3 FresnelSchlick(float HdotV, vec3 F0)
{
	return (F0 + (vec3(1.0) - F0) * pow(1.0 - HdotV, 5.0));
}

float DistributionGGX(vec3 N, vec3 H, float Roughness)
{
	float A = Roughness*Roughness;
	float A2 = A*A;
	float HdotN = max(dot(H, N), 0.0);
	float Denom = (HdotN*HdotN) * (A2 - 1.0) + 1.0;
	Denom = PI * Denom * Denom;
	
	return (A2 / Denom);
}

float GeometrySchlickGGX(float CosTheta, float Roughness)
{
	float A = Roughness + 1.0;
	float K = (A * A) / 8.0;

	return (CosTheta / (CosTheta * (1.0 - K) + K));
}
float GeometrySmith(vec3 N, vec3 V, vec3 L, float Roughness)
{
	float A = Roughness * Roughness;
	float GGX1 = GeometrySchlickGGX(max(dot(V, N), 0.0), A);
	float GGX2 = GeometrySchlickGGX(max(dot(L, N), 0.0), A);

	return (GGX1 * GGX2);
}

void main()
{
	vec3 N = normalize(Normal);
	vec3 V = normalize(CamPos - FragPosWorld);

	vec3 F0 = vec3(0.04);
	F0 = mix(F0, Albedo, Metallic);

	vec3 RadianceOut = vec3(0.0);
	for(int I = 0; I < 4; I++)
	{
		vec3 L = normalize(LightPositions[I] - FragPosWorld);
		vec3 H = normalize(V + L);

		float Distance = length(LightPositions[I] - FragPosWorld);
		float Attenuation = 1.0 / (Distance * Distance);
		vec3 LightRadiance = LightColors[I] * Attenuation;

		float NDF = DistributionGGX(N, H, Roughness);
		float G = GeometrySmith(N, V, L, Roughness);
		vec3 F = FresnelSchlick(max(dot(H, V), 0.0), F0);

		vec3 SpecularRatio = F;
		vec3 DiffuseRatio = vec3(1.0) - SpecularRatio;
		DiffuseRatio *= 1.0 - Metallic;

		vec3 Numerator = NDF * G * F;
		float Denominator = 4.0 * max(dot(V, N), 0.0) * max(dot(L, N), 0.0);
		vec3 Specular = Numerator / max(Denominator, 0.001);

		RadianceOut += (DiffuseRatio * Albedo / PI + Specular) * LightRadiance * max(dot(N, L), 0.0);
	}

	vec3 Ambient = 0.03 * Albedo * AO;
	vec3 Color = Ambient + RadianceOut;

	Color = Color / (Color + vec3(1.0));
	Color = sqrt(Color);

	FragColor = vec4(Color, 1.0);
}