#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoords;

out vec2 TexCoords;

uniform mat4 Model = mat4(1.0);
uniform mat4 View = mat4(1.0);
uniform mat4 Projection = mat4(1.0);

void main()
{
	TexCoords = aTexCoords;
	gl_Position = Projection * View * Model * vec4(aPos, 1.0);
}