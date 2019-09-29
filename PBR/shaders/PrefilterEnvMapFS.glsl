#version 330 core
out vec4 FragColor;

in vec3 LocalPos;

uniform samplerCube EnvironmentMap;
uniform float Roughness;

const float PI = 3.14159265359;

float DistributionGGX(vec3 N, vec3 H, float Roughness)
{
	float A = Roughness*Roughness;
	float A2 = A*A;
	float HdotN = max(dot(H, N), 0.0);
	float Denom = (HdotN*HdotN) * (A2 - 1.0) + 1.0;
	Denom = PI * Denom * Denom;
	
	return (A2 / Denom);
}

float VanDerCorpusSequence(uint Bits) 
{
    Bits = (Bits << 16u) | (Bits >> 16u);
    Bits = ((Bits & 0x55555555u) << 1u) | ((Bits & 0xAAAAAAAAu) >> 1u);
    Bits = ((Bits & 0x33333333u) << 2u) | ((Bits & 0xCCCCCCCCu) >> 2u);
    Bits = ((Bits & 0x0F0F0F0Fu) << 4u) | ((Bits & 0xF0F0F0F0u) >> 4u);
    Bits = ((Bits & 0x00FF00FFu) << 8u) | ((Bits & 0xFF00FF00u) >> 8u);
    return (float(Bits) * 2.3283064365386963e-10);
}

vec2 HammersleySequence(uint I, uint N)
{
	return (vec2(float(I)/float(N), VanDerCorpusSequence(I)));
}

vec3 ImportanceSampleGGX(vec2 Sample, vec3 N, float Roughness)
{
	float A = Roughness*Roughness;

	float Phi = 2.0 * PI * Sample.x;
	float CosTheta = sqrt((1.0 - Sample.y) / (1.0 + (A*A - 1.0) * Sample.y));
	float SinTheta = sqrt(1.0 - CosTheta*CosTheta);

	vec3 Cartesian;
	Cartesian.x = cos(Phi)*SinTheta;
	Cartesian.y = sin(Phi)*SinTheta;
	Cartesian.z = CosTheta;

	vec3 Up = abs(N.z) < 0.999 ? vec3(0.0, 0.0, 1.0) : vec3(1.0, 0.0, 0.0);
	vec3 Tangent = normalize(cross(Up, N));
	vec3 Bitangent = cross(N, Tangent);

	vec3 SampleVec = Tangent * Cartesian.x + Bitangent * Cartesian.y + N * Cartesian.z;
	return (normalize(SampleVec));
}

void main()
{
	vec3 N = normalize(LocalPos);
	vec3 V = N;

	uint SampleCount = 1024u;
	float TotalWeight = 0.0;
	vec3 PrefilteredColor = vec3(0.0);
	for(uint I = 0u; I < SampleCount; I++)
	{
		vec2 Sample = HammersleySequence(I, SampleCount);
		vec3 H = ImportanceSampleGGX(Sample, N, Roughness);
		vec3 L = normalize(2.0*dot(V, H)*H - V);
		
		float Weight = max(dot(N, L), 0.0);
		if(Weight > 0.0)
		{
			float D = DistributionGGX(N, H, Roughness);
			float NdotH = max(dot(N, H), 0.0);
			float HdotV = max(dot(H, V), 0.0);
			float PDF = D * NdotH / (4.0 * HdotV) + 0.0001;

			float Resolution = 512.0;
			float Texel = 4.0 * PI / (6.0 * Resolution * Resolution);
			float saSample = 1.0 / (float(SampleCount) * PDF + 0.0001);

			float MipLevel = (Roughness == 0) ? 0.0 : 0.5 * log2(saSample / Texel);

			PrefilteredColor += textureLod(EnvironmentMap, L, MipLevel).rgb*Weight;
			TotalWeight += Weight;
		}
	}

	PrefilteredColor = PrefilteredColor / TotalWeight;

	FragColor = vec4(PrefilteredColor, 1.0);
}