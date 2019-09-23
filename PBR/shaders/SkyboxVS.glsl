#version 330 core
layout (location = 0) in vec3 aPos;

uniform mat4 View;
uniform mat4 Projection;

out vec3 LocalPos;

void main()
{
	LocalPos = aPos;

	mat4 SkyboxView = mat4(mat3(View));
	vec4 ClipPos = Projection * SkyboxView * vec4(LocalPos, 1.0);

	gl_Position = ClipPos.xyww;
}