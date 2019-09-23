#version 330 core
layout (location = 0) in vec3 aPos;

out vec3 LocalPos;

uniform mat4 View = mat4(1.0);
uniform mat4 Projection = mat4(1.0);

void main()
{
	LocalPos = aPos;

	gl_Position = Projection * View * vec4(LocalPos, 1.0);
}