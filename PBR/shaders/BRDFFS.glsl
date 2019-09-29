#version 330 core
out vec2 FragColor;

in vec2 TexCoords;

const float PI = 3.14159265359;

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

float GeometrySchlickGGX(float CosTheta, float Roughness)
{
	float A = Roughness;
	float K = (A * A) / 2.0;

	return (CosTheta / (CosTheta * (1.0 - K) + K));
}
float GeometrySmith(vec3 N, vec3 V, vec3 L, float Roughness)
{
	float GGX1 = GeometrySchlickGGX(max(dot(V, N), 0.0), Roughness);
	float GGX2 = GeometrySchlickGGX(max(dot(L, N), 0.0), Roughness);

	return (GGX1 * GGX2);
}

vec2 IntegrateBRDF(float NdotV, float Roughness)
{
	vec3 V;
	V.x = sqrt(1.0 - NdotV*NdotV);
	V.y = 0.0;
	V.z = NdotV;

	vec3 N = vec3(0.0, 0.0, 1.0);

	float A = 0.0;
	float B = 0.0;

	uint SampleCount = 1024u;
	for(uint I = 0u; I < SampleCount; I++)
	{
		vec2 Sample = HammersleySequence(I, SampleCount);
		vec3 H = ImportanceSampleGGX(Sample, N, Roughness);
		vec3 L = normalize(2.0*dot(V, H)*H - V);

		float NdotL = max(L.z, 0.0);
        float NdotH = max(H.z, 0.0);
        float VdotH = max(dot(V, H), 0.0);

		if(NdotL > 0.0)
		{
			float G = GeometrySmith(N, V, L, Roughness);
			float G2 = (G * VdotH) / (NdotH * NdotV);
			float FTerm = pow(1.0 - VdotH, 5.0);
			
			A += (1.0 - FTerm) * G2;
			B += FTerm * G2;
		}
	}

	A /= float(SampleCount);
	B /= float(SampleCount);

	return(vec2(A, B));
}

void main()
{
	vec2 BRDF = IntegrateBRDF(TexCoords.x, TexCoords.y);
	FragColor = BRDF;
}