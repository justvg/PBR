#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoords;
layout (location = 2) in vec3 aNormal;

out vec2 TexCoords;
out vec3 FragPosWorld;
out vec3 Normal;

uniform mat4 Model = mat4(1.0);
uniform mat4 View = mat4(1.0);
uniform mat4 Projection = mat4(1.0);

void main()
{
	TexCoords = aTexCoords;
	FragPosWorld = vec3(Model * vec4(aPos, 1.0));
	Normal = mat3(Model) * aNormal;

	gl_Position = Projection * View * vec4(FragPosWorld, 1.0);
}