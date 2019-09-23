#include <GL\glew.h>
#include <GLFW\glfw3.h>

#include <Windows.h>
#include <iostream>
#include <vector>
#include <string>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define internal static
#define global_variable static 
#define local_persist static

#define ArrayCount(Array) (sizeof(Array) / sizeof(Array[0]))

#include "math.hpp"
#include "main.h"

global_variable LARGE_INTEGER GlobalPerfCounterFrequency;

struct read_entire_file_result
{
	void *Memory;
	uint32_t Size;
};
internal read_entire_file_result
ReadEntireFile(char *Filename)
{
	read_entire_file_result Result;

	FILE *File = fopen(Filename, "rb");

	fseek(File, 0, SEEK_END);
	Result.Size = ftell(File);
	fseek(File, 0, SEEK_SET);

	Result.Memory = malloc(Result.Size);
	fread(Result.Memory, 1, Result.Size, File);
	fclose(File);

	return(Result);
}

struct shader
{
	uint32_t ID;
};
internal void
CompileShader(shader *Shader, char *VertexPath, char *FragmentPath)
{
	uint32_t VS = glCreateShader(GL_VERTEX_SHADER);
	uint32_t FS = glCreateShader(GL_FRAGMENT_SHADER);
	Shader->ID = glCreateProgram();

	read_entire_file_result VSSourceCode = ReadEntireFile(VertexPath);
	read_entire_file_result FSSourceCode = ReadEntireFile(FragmentPath);

	int32_t Success;
	char InfoLog[1024];

	glShaderSource(VS, 1, (char **)&VSSourceCode.Memory, (GLint *)&VSSourceCode.Size);
	glCompileShader(VS);
	glGetShaderiv(VS, GL_COMPILE_STATUS, &Success);
	if (!Success)
	{
		glGetShaderInfoLog(VS, sizeof(InfoLog), 0, InfoLog);
		std::cout << "ERROR::SHADER_COMPILATION_ERROR of type: " << "VS\n" << InfoLog << "\n";
	}

	glShaderSource(FS, 1, (char **)&FSSourceCode.Memory, (GLint *)&FSSourceCode.Size);
	glCompileShader(FS);
	glGetShaderiv(FS, GL_COMPILE_STATUS, &Success);
	if (!Success)
	{
		glGetShaderInfoLog(FS, sizeof(InfoLog), 0, InfoLog);
		std::cout << "ERROR::SHADER_COMPILATION_ERROR of type: " << "FS\n" << InfoLog << "\n";
	}

	glAttachShader(Shader->ID, VS);
	glAttachShader(Shader->ID, FS);
	glLinkProgram(Shader->ID);
	glGetProgramiv(Shader->ID, GL_LINK_STATUS, &Success);
	if (!Success)
	{
		glGetProgramInfoLog(Shader->ID, sizeof(InfoLog), 0, InfoLog);
		std::cout << "ERROR::PROGRAM_LINKING_ERROR of type: " << "Program" << "\n" << InfoLog << "\n";
	}

	free(VSSourceCode.Memory);
	free(FSSourceCode.Memory);
	glDeleteShader(VS);
	glDeleteShader(FS);
}

inline void
UseShader(shader Shader)
{
	glUseProgram(Shader.ID);
}

inline void 
SetFloat(shader Shader, char *Name, real32 Value)
{
	glUniform1f(glGetUniformLocation(Shader.ID, Name), Value);
}

inline void
SetInt(shader Shader, char *Name, int32_t Value)
{
	glUniform1i(glGetUniformLocation(Shader.ID, Name), Value);
}

inline void
SetVec3(shader Shader, char *Name, vec3 Value)
{
	glUniform3fv(glGetUniformLocation(Shader.ID, Name), 1, (GLfloat *)&Value.m);
}

inline void
SetVec4(shader Shader, char *Name, vec4 Value)
{
	glUniform4fv(glGetUniformLocation(Shader.ID, Name), 1, (GLfloat *)&Value.m);
}

inline void
SetMat4(shader Shader, char *Name, mat4 Value)
{
	glUniformMatrix4fv(glGetUniformLocation(Shader.ID, Name), 1, GL_FALSE, (GLfloat *)&Value.FirstColumn);
}

struct engine_input
{
	bool MoveForward;
	bool MoveBack;
	bool MoveRight;
	bool MoveLeft;

	int32_t MouseX, MouseY;
};

struct camera
{
	vec3 P;
	vec3 TargetDir;

	real32 Pitch, Head;
};

internal void 
GLFWKeyCallback(GLFWwindow *Window, int32_t Key, int32_t Scancode, int32_t Action, int32_t Mods)
{
	engine_input *Input = (engine_input *)glfwGetWindowUserPointer(Window);
	
	if (Key == GLFW_KEY_W)
	{
		Input->MoveForward = (Action != GLFW_RELEASE);
	}
	else if (Key == GLFW_KEY_S)
	{
		Input->MoveBack = (Action != GLFW_RELEASE);
	}
	else if (Key == GLFW_KEY_D)
	{
		Input->MoveRight = (Action != GLFW_RELEASE);
	}
	else if (Key == GLFW_KEY_A)
	{
		Input->MoveLeft = (Action != GLFW_RELEASE);
	}	
}

internal void 
GLFWCursorCallback(GLFWwindow *Window, real64 X, real64 Y)
{
	engine_input *Input = (engine_input *)glfwGetWindowUserPointer(Window);

	Input->MouseX = (int32_t)X;
	Input->MouseY = (int32_t)Y;
}

internal void 
ProcessInput(engine_input *Input, camera *Camera, real32 dt)
{
	int32_t X = Input->MouseX;
	int32_t Y = Input->MouseY;

	local_persist int32_t LastX;
	local_persist int32_t LastY;
	local_persist bool FirstMouse = true;
	if (FirstMouse)
	{
		FirstMouse = false;
		LastX = X;
		LastY = Y;
	}

	real32 RotSensetivity = 0.05f;
	Camera->Head += (X - LastX)*RotSensetivity;
	Camera->Pitch += (LastY - Y)*RotSensetivity;

	LastX = X;
	LastY = Y;

	Camera->Pitch = Camera->Pitch > 89.0f ? 89.0f : Camera->Pitch;
	Camera->Pitch = Camera->Pitch < -89.0f ? -89.0f : Camera->Pitch;

	real32 CameraTargetDirX = sinf(DEG2RAD(Camera->Head))*cosf(DEG2RAD(Camera->Pitch));
	real32 CameraTargetDirY = sinf(DEG2RAD(Camera->Pitch));
	real32 CameraTargetDirZ = -cosf(DEG2RAD(Camera->Head))*cosf(DEG2RAD(Camera->Pitch));
	Camera->TargetDir = Normalize(vec3(CameraTargetDirX, CameraTargetDirY, CameraTargetDirZ));

	vec3 CameraFront = Camera->TargetDir;
	vec3 CameraRight = Normalize(Cross(Camera->TargetDir, vec3(0.0f, 1.0f, 0.0f)));

	real32 Speed = 5.0f;
	if (Input->MoveForward)
	{
		Camera->P += Speed*CameraFront*dt;
	}
	if (Input->MoveBack)
	{
		Camera->P -= Speed*CameraFront*dt;
	}
	if (Input->MoveRight)
	{
		Camera->P += Speed*CameraRight*dt;
	}
	if (Input->MoveLeft)
	{
		Camera->P -= Speed*CameraRight*dt;
	}
}

inline LARGE_INTEGER
GetWallClock(void)
{
	LARGE_INTEGER Result;
	QueryPerformanceCounter(&Result);
	return(Result);
}

inline real32
GetSecondsElapsed(LARGE_INTEGER Start, LARGE_INTEGER End)
{
	real32 Result = (End.QuadPart - Start.QuadPart) / (real32)GlobalPerfCounterFrequency.QuadPart;
	return(Result);
}

global_variable GLuint SphereVAO = 0;
global_variable uint32_t SphereIndexCount = 0;
void RenderSphere(void)
{
	if (SphereVAO == 0)
	{
		glGenVertexArrays(1, &SphereVAO);

		GLuint VBOPos, VBOUV, VBONormals, EBO;
		glGenBuffers(1, &VBOPos);
		glGenBuffers(1, &VBOUV);
		glGenBuffers(1, &VBONormals);
		glGenBuffers(1, &EBO);

		std::vector<vec3> Positions;
		std::vector<vec2> UV;
		std::vector<vec3> Normals;
		std::vector<uint32_t> Indices;

#define X_SEGMENTS 64
#define Y_SEGMENTS 64
		for (uint32_t Y = 0; Y <= Y_SEGMENTS; Y++)
		{
			for (uint32_t X = 0; X <= X_SEGMENTS; X++)
			{
				real32 XSegment = X / (real32)X_SEGMENTS;
				real32 YSegment = Y / (real32)Y_SEGMENTS;
				real32 XPos = cosf(XSegment * 2.0f * PI) * sinf(YSegment * PI);
				real32 YPos = cosf(YSegment * PI);
				real32 ZPos = sinf(XSegment * 2.0f * PI) * sinf(YSegment * PI);

				Positions.push_back(vec3(XPos, YPos, ZPos));
				UV.push_back(vec2(XSegment, YSegment));
				Normals.push_back(vec3(XPos, YPos, ZPos));
			}
		}

		bool OddRow = false;
		for (int Y = 0; Y < Y_SEGMENTS; ++Y)
		{
			if (!OddRow)
			{
				for (int X = 0; X <= X_SEGMENTS; ++X)
				{
					Indices.push_back(Y * (X_SEGMENTS + 1) + X);
					Indices.push_back((Y + 1) * (X_SEGMENTS + 1) + X);
				}
			}
			else
			{
				for (int X = X_SEGMENTS; X >= 0; --X)
				{
					Indices.push_back((Y + 1) * (X_SEGMENTS + 1) + X);
					Indices.push_back(Y * (X_SEGMENTS + 1) + X);
				}
			}
			OddRow = !OddRow;
		}
		SphereIndexCount = (uint32_t)Indices.size();

		glBindVertexArray(SphereVAO);

		glBindBuffer(GL_ARRAY_BUFFER, VBOPos);
		glBufferData(GL_ARRAY_BUFFER, Positions.size() * sizeof(vec3), &Positions[0], GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vec3), (void *)0);

		glBindBuffer(GL_ARRAY_BUFFER, VBOUV);
		glBufferData(GL_ARRAY_BUFFER, UV.size() * sizeof(vec2), &UV[0], GL_STATIC_DRAW);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(vec2), (void *)0);

		glBindBuffer(GL_ARRAY_BUFFER, VBONormals);
		glBufferData(GL_ARRAY_BUFFER, Normals.size() * sizeof(vec3), &Normals[0], GL_STATIC_DRAW);
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(vec3), (void *)0);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, Indices.size() * sizeof(uint32_t), &Indices[0], GL_STATIC_DRAW);
	}

	glBindVertexArray(SphereVAO);
	glDrawElements(GL_TRIANGLE_STRIP, SphereIndexCount, GL_UNSIGNED_INT, 0);
}

int main(void)
{
	QueryPerformanceFrequency(&GlobalPerfCounterFrequency);

	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_SAMPLES, 4);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	GLFWmonitor *Monitor = glfwGetPrimaryMonitor();
	const GLFWvidmode *VidMode = glfwGetVideoMode(Monitor);
	int32_t MonitorRefreshRate = VidMode->refreshRate;
	real32 GameUpdateHz = (real32)MonitorRefreshRate;
	real32 TargetSecondsPerFrame = 1.0f / GameUpdateHz;

	uint32_t Width = 960, Height = 540;
	GLFWwindow *Window = glfwCreateWindow(Width, Height, "PBR", 0, 0);
	glfwMakeContextCurrent(Window);
	engine_input Input = {};
	glfwSwapInterval(0);
	glfwSetWindowUserPointer(Window, &Input);
	glfwSetInputMode(Window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	glfwSetKeyCallback(Window, GLFWKeyCallback);
	glfwSetCursorPosCallback(Window, GLFWCursorCallback);
	glViewport(0, 0, Width, Height);
	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_MULTISAMPLE);
	
	glewInit();

	real32 CubeVertices[] = {
		// Back face
		-1.0f, -1.0f, -1.0f,  
		1.0f,  1.0f, -1.0f,  
		1.0f, -1.0f, -1.0f,  
		1.0f,  1.0f, -1.0f,  
		-1.0f, -1.0f, -1.0f, 
		-1.0f,  1.0f, -1.0f, 
		// Front face
		-1.0f, -1.0f,  1.0f,  
		1.0f, -1.0f,  1.0f,  
		1.0f,  1.0f,  1.0f,  
		1.0f,  1.0f,  1.0f,  
		-1.0f,  1.0f,  1.0f, 
		-1.0f, -1.0f,  1.0f, 
		// Left face
		-1.0f,  1.0f,  1.0f, 
		-1.0f,  1.0f, -1.0f, 
		-1.0f, -1.0f, -1.0f, 
		-1.0f, -1.0f, -1.0f, 
		-1.0f, -1.0f,  1.0f, 
		-1.0f,  1.0f,  1.0f, 
		// Right face
		1.0f,  1.0f,  1.0f,  
		1.0f, -1.0f, -1.0f,  
		1.0f,  1.0f, -1.0f,  
		1.0f, -1.0f, -1.0f,  
		1.0f,  1.0f,  1.0f,  
		1.0f, -1.0f,  1.0f,  
		// Bottom face
		-1.0f, -1.0f, -1.0f, 
		1.0f, -1.0f, -1.0f,  
		1.0f, -1.0f,  1.0f,  
		1.0f, -1.0f,  1.0f,  
		-1.0f, -1.0f,  1.0f, 
		-1.0f, -1.0f, -1.0f, 
		// Top face
		-1.0f,  1.0f, -1.0f, 
		1.0f,  1.0f , 1.0f,  
		1.0f,  1.0f, -1.0f,  
		1.0f,  1.0f,  1.0f,  
		-1.0f,  1.0f, -1.0f, 
		-1.0f,  1.0f,  1.0f, 
	};
	GLuint CubeVAO, CubeVBO;
	glGenVertexArrays(1, &CubeVAO);
	glGenBuffers(1, &CubeVBO);
	glBindVertexArray(CubeVAO);
	glBindBuffer(GL_ARRAY_BUFFER, CubeVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(CubeVertices), CubeVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3*sizeof(real32), (void *)0);
	glBindVertexArray(0);

	shader PBRShader, EquirectangularToCubemapShader, ConvolutionIrradianceShader, SkyboxShader;
	CompileShader(&PBRShader, "shaders/PBRVS.glsl", "shaders/PBRFS.glsl");
	CompileShader(&EquirectangularToCubemapShader, "shaders/EquirectangularToCubemapVS.glsl",
												   "shaders/EquirectangularToCubemapFS.glsl");
	CompileShader(&ConvolutionIrradianceShader, "shaders/EquirectangularToCubemapVS.glsl",
												"shaders/ConvoluteIrradianceFS.glsl");
	CompileShader(&SkyboxShader, "shaders/SkyboxVS.glsl", "shaders/SkyboxFS.glsl");

	stbi_set_flip_vertically_on_load(true);
	int32_t EnvWidth, EnvHeight, Components;
	real32 *Data = stbi_loadf("Data/Newport_Loft_Ref.hdr", &EnvWidth, &EnvHeight, &Components, 0);
	GLuint HDRTexture;
	if (Data)
	{
		glGenTextures(1, &HDRTexture);
		glBindTexture(GL_TEXTURE_2D, HDRTexture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, EnvWidth, EnvHeight, 0, GL_RGB, GL_FLOAT, Data);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		stbi_image_free(Data);
	}
	else
	{
		std::cout << "Can't load HDR image." << std::endl;
	}


	// NOTE(georgy): Equirectangular to cube map
	GLuint CaptureFBO, CaptureRBO;
	glGenFramebuffers(1, &CaptureFBO);
	glGenRenderbuffers(1, &CaptureRBO);

	glBindFramebuffer(GL_FRAMEBUFFER, CaptureFBO);
	glBindRenderbuffer(GL_RENDERBUFFER, CaptureRBO);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 512, 512);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, CaptureRBO);

	GLuint EnvCubemap;
	glGenTextures(1, &EnvCubemap);
	glBindTexture(GL_TEXTURE_CUBE_MAP, EnvCubemap);
	for (uint32_t I = 0; I < 6; I++)
	{
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + I, 0, GL_RGB16F, 512, 512, 0, GL_RGB, GL_FLOAT, 0);
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	
	mat4 CaptureProjection = Perspective(90.0f, 1.0f, 0.1f, 10.0f);
	mat4 CaptureViews[] = 
	{
		LookAt(vec3(0.0f, 0.0f, 0.0f), vec3(1.0f,  0.0f,  0.0f), vec3(0.0f, -1.0f,  0.0f)),
		LookAt(vec3(0.0f, 0.0f, 0.0f), vec3(-1.0f,  0.0f,  0.0f), vec3(0.0f, -1.0f,  0.0f)),
		LookAt(vec3(0.0f, 0.0f, 0.0f), vec3(0.0f,  1.0f,  0.0f), vec3(0.0f,  0.0f,  1.0f)),
		LookAt(vec3(0.0f, 0.0f, 0.0f),vec3(0.0f, -1.0f,  0.0f), vec3(0.0f,  0.0f, -1.0f)),
		LookAt(vec3(0.0f, 0.0f, 0.0f), vec3(0.0f,  0.0f,  1.0f), vec3(0.0f, -1.0f,  0.0f)),
		LookAt(vec3(0.0f, 0.0f, 0.0f), vec3(0.0f,  0.0f, -1.0f), vec3(0.0f, -1.0f,  0.0))
	};

	UseShader(EquirectangularToCubemapShader);
	SetMat4(EquirectangularToCubemapShader, "Projection", CaptureProjection);
	SetInt(EquirectangularToCubemapShader, "EquirectangularMap", 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, HDRTexture);

	glViewport(0, 0, 512, 512);
	glBindFramebuffer(GL_FRAMEBUFFER, CaptureFBO);
	for (uint32_t I = 0; I < 6; I++)
	{
		SetMat4(EquirectangularToCubemapShader, "View", CaptureViews[I]);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + I, EnvCubemap, 0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glBindVertexArray(CubeVAO);
		glDrawArrays(GL_TRIANGLES, 0, 36);
	}
	

	// NOTE(georgy): Diffuse irradiance map
	GLuint IrradianceMap;
	glGenTextures(1, &IrradianceMap);
	glBindTexture(GL_TEXTURE_CUBE_MAP, IrradianceMap);
	for (uint32_t I = 0; I < 6; I++)
	{
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + I, 0, GL_RGB16F, 32, 32, 0, GL_RGB, GL_FLOAT, 0);
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glBindFramebuffer(GL_FRAMEBUFFER, CaptureFBO);
	glBindRenderbuffer(GL_RENDERBUFFER, CaptureRBO);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 32, 32);

	UseShader(ConvolutionIrradianceShader);
	SetInt(ConvolutionIrradianceShader, "EnvironmentMap", 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, EnvCubemap);
	SetMat4(ConvolutionIrradianceShader, "Projection", CaptureProjection);

	glViewport(0, 0, 32, 32);
	for (uint32_t I = 0; I < 6; I++)
	{
		SetMat4(ConvolutionIrradianceShader, "View", CaptureViews[I]);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, 
							   GL_TEXTURE_CUBE_MAP_POSITIVE_X + I, IrradianceMap, 0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glBindVertexArray(CubeVAO);
		glDrawArrays(GL_TRIANGLES, 0, 36);
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, Width, Height);


	mat4 PerspectiveProjection = Perspective(45.0f, (real32)Width / (real32)Height, 0.1f, 100.0f);
	UseShader(SkyboxShader);
	SetInt(SkyboxShader, "Skybox", 0);
	SetMat4(SkyboxShader, "Projection", PerspectiveProjection);

	UseShader(PBRShader);
	SetInt(PBRShader, "IrradianceMap", 0);
	SetMat4(PBRShader, "Projection", PerspectiveProjection);

	camera Camera = {};
	Camera.P = vec3(0.0f, 0.0f, 3.0f);
	Camera.TargetDir = vec3(0.0f, 0.0f, -1.0f);

	SetVec3(PBRShader, "Albedo", vec3(0.5f, 0.0f, 0.0f));
	SetFloat(PBRShader, "AO", 1.0f);

	vec3 LightPositions[] = 
	{
		vec3(-10.0f, 10.0f, 10.0f),
		vec3(10.0f, 10.0f, 10.0f),
		vec3(-10.0f, -10.0f, 10.0f),
		vec3(10.0f, -10.0f, 10.0f),
	};
	vec3 LightColors[] = 
	{
		vec3(300.0f, 300.0f, 300.0f),
		vec3(300.0f, 300.0f, 300.0f),
		vec3(300.0f, 300.0f, 300.0f),
		vec3(300.0f, 300.0f, 300.0f)
	};
	uint32_t Rows = 7;
	uint32_t Columns = 7;
	real32 Spacing = 2.5f;

	LARGE_INTEGER LastCounter;
	QueryPerformanceCounter(&LastCounter);
	while (!glfwWindowShouldClose(Window))
	{
		glfwPollEvents();
		ProcessInput(&Input, &Camera, TargetSecondsPerFrame);

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		UseShader(PBRShader);
		mat4 View = LookAt(Camera.P, Camera.P + Camera.TargetDir);
		SetMat4(PBRShader, "View", View);
		SetVec3(PBRShader, "CamPos", Camera.P);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, IrradianceMap);

		mat4 Model = Identity();
		for (uint32_t Row = 0; Row < Rows; Row++)
		{
			SetFloat(PBRShader, "Metallic", Row / (real32)Rows);
			for (uint32_t Column = 0; Column < Columns; Column++)
			{
				SetFloat(PBRShader, "Roughness", Clamp(Column / (real32)Columns, 0.05f, 1.0f));

				Model = Translate(vec3((Column - (Columns / 2.0f)) * Spacing,
									   (Row - (Rows / 2.0f)) * Spacing,
										0.0f));
				SetMat4(PBRShader, "Model", Model);
				RenderSphere();
			}

		}

		for (uint32_t I = 0; I < ArrayCount(LightPositions); I++)
		{
			SetVec3(PBRShader, (char *)("LightPositions[" + std::to_string(I) + "]").c_str(), LightPositions[I]);
			SetVec3(PBRShader, (char *)("LightColors[" + std::to_string(I) + "]").c_str(), LightColors[I]);

			Model = Translate(LightPositions[I]);
			SetMat4(PBRShader, "Model", Model);
			RenderSphere();
		}

		glDepthFunc(GL_LEQUAL);
		UseShader(SkyboxShader);
		SetMat4(SkyboxShader, "View", View);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, EnvCubemap);
		glBindVertexArray(CubeVAO);
		glDrawArrays(GL_TRIANGLES, 0, 36);
		glDepthFunc(GL_LESS);

		real32 SecondsElapsedForFrame = GetSecondsElapsed(LastCounter, GetWallClock());
		while (SecondsElapsedForFrame < TargetSecondsPerFrame)
		{
			SecondsElapsedForFrame = GetSecondsElapsed(LastCounter, GetWallClock());
		}
		LastCounter = GetWallClock();

		glfwSwapBuffers(Window);
	}

	return(0);
}