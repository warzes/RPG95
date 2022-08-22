#include "stdafx.h"
#include "Engine.h"
#include "Timer.h"
//-----------------------------------------------------------------------------
#if defined(_MSC_VER)
#	pragma comment( lib, "SDL2.lib" )
#	pragma comment( lib, "SDL2main.lib" )
#endif
//-----------------------------------------------------------------------------
SDL_Window* window = nullptr;
SDL_Renderer* renderer = nullptr;
#if USE_OPENGL
SDL_GLContext gContext = nullptr;
#endif


PixelBuffer<VIEWPORT_X, VIEWPORT_Y>* gPixelBuffer = nullptr;
Input gInput;
constexpr float MicrosecondsToSeconds = 1.0f / 1000000.0f;
std::chrono::steady_clock::time_point startTime;
int64_t frameTimeCurrent = 0;
int64_t frameTimeLast = 0;
int64_t frameTimeDelta = 0;
float deltaTime = 0.0f;
SDL_Event sdlEvent = {};
bool running = true;

timer ttt(true);

#if USE_OPENGL
//Render flag
bool gRenderQuad = true;

//Graphics program
GLuint gProgramID = 0;
GLint gVertexPos2DLocation = -1;
GLuint gVAO = 0;
GLuint gVBO = 0;
GLuint gIBO = 0;

GLuint pbo = 0;

struct Point
{
	float x;
	float y;
};

struct vertex_t
{
	Point pos;
	Point tex;
};

const int    IMAGE_WIDTH = 2048;
const int    IMAGE_HEIGHT = 2048;
const int    CHANNEL_COUNT = 4;
const int    DATA_SIZE = IMAGE_WIDTH * IMAGE_HEIGHT * CHANNEL_COUNT;
const GLenum PIXEL_FORMAT = GL_BGRA;

GLuint pboIds[2];                   // IDs of PBO
GLuint textureId;                   // ID of texture
GLubyte* imageData = 0;             // pointer to texture buffer

void printProgramLog(GLuint program)
{
	//Make sure name is shader
	if (glIsProgram(program))
	{
		//Program log length
		int infoLogLength = 0;
		int maxLength = infoLogLength;

		//Get info string length
		glGetProgramiv(program, GL_INFO_LOG_LENGTH, &maxLength);

		//Allocate string
		char* infoLog = new char[maxLength];

		//Get info log
		glGetProgramInfoLog(program, maxLength, &infoLogLength, infoLog);
		if (infoLogLength > 0)
		{
			//Print Log
			printf("%s\n", infoLog);
		}

		//Deallocate string
		delete[] infoLog;
	}
	else
	{
		printf("Name %d is not a program\n", program);
	}
}

void printShaderLog(GLuint shader)
{
	//Make sure name is shader
	if (glIsShader(shader))
	{
		//Shader log length
		int infoLogLength = 0;
		int maxLength = infoLogLength;

		//Get info string length
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &maxLength);

		//Allocate string
		char* infoLog = new char[maxLength];

		//Get info log
		glGetShaderInfoLog(shader, maxLength, &infoLogLength, infoLog);
		if (infoLogLength > 0)
		{
			//Print Log
			printf("%s\n", infoLog);
		}

		//Deallocate string
		delete[] infoLog;
	}
	else
	{
		printf("Name %d is not a shader\n", shader);
	}
}

bool initGL()
{
	//Success flag
	bool success = true;

	//Generate program
	gProgramID = glCreateProgram();

	//Create vertex shader
	GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);

	//Get vertex source
	const GLchar* vertexShaderSource[] =
	{
		"#version 330\n layout(location = 0) in vec2 LVertexPos2D; layout(location = 1) in vec2 vertexUV; out vec2 UV; void main() { gl_Position = vec4( LVertexPos2D.x, LVertexPos2D.y, 0, 1 ); UV = vertexUV; }"
	};

	//Set vertex source
	glShaderSource(vertexShader, 1, vertexShaderSource, NULL);

	//Compile vertex source
	glCompileShader(vertexShader);

	//Check vertex shader for errors
	GLint vShaderCompiled = GL_FALSE;
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &vShaderCompiled);
	if (vShaderCompiled != GL_TRUE)
	{
		printf("Unable to compile vertex shader %d!\n", vertexShader);
		printShaderLog(vertexShader);
		success = false;
	}
	else
	{
		//Attach vertex shader to program
		glAttachShader(gProgramID, vertexShader);


		//Create fragment shader
		GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);

		//Get fragment source
		const GLchar* fragmentShaderSource[] =
		{
			"#version 330 core\n in vec2 UV; out vec3 LFragment; uniform sampler2D textureSampler; void main() { LFragment = vec3( 1.0, 1.0, 1.0); LFragment = texture( textureSampler, UV ).rgb;}"
		};

		//Set fragment source
		glShaderSource(fragmentShader, 1, fragmentShaderSource, NULL);

		//Compile fragment source
		glCompileShader(fragmentShader);

		//Check fragment shader for errors
		GLint fShaderCompiled = GL_FALSE;
		glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &fShaderCompiled);
		if (fShaderCompiled != GL_TRUE)
		{
			printf("Unable to compile fragment shader %d!\n", fragmentShader);
			printShaderLog(fragmentShader);
			success = false;
		}
		else
		{
			//Attach fragment shader to program
			glAttachShader(gProgramID, fragmentShader);


			//Link program
			glLinkProgram(gProgramID);

			//Check for errors
			GLint programSuccess = GL_TRUE;
			glGetProgramiv(gProgramID, GL_LINK_STATUS, &programSuccess);
			if (programSuccess != GL_TRUE)
			{
				printf("Error linking program %d!\n", gProgramID);
				printProgramLog(gProgramID);
				success = false;
			}
			else
			{
				//Get vertex attribute location
				gVertexPos2DLocation = glGetAttribLocation(gProgramID, "LVertexPos2D");
				if (gVertexPos2DLocation == -1)
				{
					printf("LVertexPos2D is not a valid glsl program variable!\n");
					success = false;
				}
				else
				{
					//Initialize clear color
					glClearColor(0.f, 0.f, 0.f, 1.f);

					glGenVertexArrays(1, &gVAO);
					glBindVertexArray(gVAO);
					
					//VBO data
					GLfloat vertexData[] =
					{
						-0.5f, -0.5f, 0.0f, 0.0f,
						 0.5f, -0.5f, 1.0f, 0.0f,
						 0.5f,  0.5f, 1.0f, 1.0f,
						-0.5f,  0.5f, 0.0f, 1.0f,
					};

					//IBO data
					GLuint indexData[] = { 0, 1, 2, 2, 3, 0 };

					//Create VBO
					glGenBuffers(1, &gVBO);
					glBindBuffer(GL_ARRAY_BUFFER, gVBO);
					glBufferData(GL_ARRAY_BUFFER, 4*sizeof(vertex_t), vertexData, GL_STATIC_DRAW);

					//Create IBO
					glGenBuffers(1, &gIBO);
					glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gIBO);
					glBufferData(GL_ELEMENT_ARRAY_BUFFER, 6 * sizeof(GLuint), indexData, GL_STATIC_DRAW);

					glEnableVertexAttribArray(0);
					glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(vertex_t), (void*)offsetof(vertex_t, pos));
					glEnableVertexAttribArray(0);
					glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(vertex_t), (void*)offsetof(vertex_t, tex));
				}
			}
		}
	}

	glGenBuffers(1, &pbo);
	glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pbo);
	glBufferData(GL_PIXEL_UNPACK_BUFFER, 512 * 512 * 3, NULL, GL_STREAM_DRAW);
	void* mappedBuffer = glMapBuffer(GL_PIXEL_UNPACK_BUFFER, GL_WRITE_ONLY);

	//static int color = 0;
	//int* ptr = (int*)mappedBuffer;
	//for (int i = 0; i < 512; ++i)
	//{
	//	for (int j = 0; j < 512; ++j)
	//	{
	//		*ptr = color;
	//		++ptr;
	//	}
	//	color += 257;
	//}
	//++color;            // scroll down

	/*for (int x = 0; x < 512; x++)
	{
		for (int y = 0; y < 512; y++)
		{
			int r = rand() % 255;
			int g = rand() % 255;
			int b = rand() % 255;

			mappedBuffer
		}
	}*/


	//int width, height;
	//unsigned char* image = SOIL_load_image("image.png", &width, &height, 0, SOIL_LOAD_RGB);
	//memcpy(mappedBuffer, image, width * height * 3);
	//SOIL_free_image(image);

	//glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pbo);
	//glUnmapBuffer(GL_PIXEL_UNPACK_BUFFER);
	//glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 512, 512, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);



	glGenBuffers(2, pboIds);
	glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pboIds[0]);
	glBufferData(GL_PIXEL_UNPACK_BUFFER, DATA_SIZE, 0, GL_STREAM_DRAW);
	glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pboIds[1]);
	glBufferData(GL_PIXEL_UNPACK_BUFFER, DATA_SIZE, 0, GL_STREAM_DRAW);
	glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);

	imageData = new GLubyte[DATA_SIZE];
	memset(imageData, 0, DATA_SIZE);



	return success;
}

void updatePixels(GLubyte* dst, int size)
{
	static int color = 0;

	if (!dst)
		return;

	int* ptr = (int*)dst;

	// copy 4 bytes at once
	for (int i = 0; i < IMAGE_HEIGHT; ++i)
	{
		for (int j = 0; j < IMAGE_WIDTH; ++j)
		{
			*ptr = color;
			++ptr;
		}
		color += 257;
	}
	++color;            // scroll down
}


#endif

//-----------------------------------------------------------------------------
void UpdateFrame()
{
#if defined(__EMSCRIPTEN__)
	if (running == false) emscripten_cancel_main_loop();
#endif
	const auto curTime = std::chrono::high_resolution_clock::now();
	frameTimeCurrent = std::chrono::duration_cast<std::chrono::microseconds>(curTime - startTime).count();
	frameTimeDelta = frameTimeCurrent - frameTimeLast;
	frameTimeLast = frameTimeCurrent;
	deltaTime = static_cast<float>(frameTimeDelta) * MicrosecondsToSeconds;

#if USE_OPENGL
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// bind the texture and PBO
	glBindTexture(GL_TEXTURE_2D, textureId);
	glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pboIds[0]);

	// copy pixels from PBO to texture object
	// Use offset instead of ponter.
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, IMAGE_WIDTH, IMAGE_HEIGHT, PIXEL_FORMAT, GL_UNSIGNED_BYTE, 0);

	glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pboIds[1]);

	glBufferData(GL_PIXEL_UNPACK_BUFFER, DATA_SIZE, 0, GL_STREAM_DRAW);
	GLubyte* ptr = (GLubyte*)glMapBuffer(GL_PIXEL_UNPACK_BUFFER, GL_WRITE_ONLY);
	if (ptr)
	{
		// update data directly on the mapped buffer
		updatePixels(ptr, DATA_SIZE);
		glUnmapBuffer(GL_PIXEL_UNPACK_BUFFER);  // release pointer to mapping buffer
	}
	glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);


	glBindTexture(GL_TEXTURE_2D, textureId);



	glUseProgram(gProgramID);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);

	////Enable vertex position
	//glEnableVertexAttribArray(gVertexPos2DLocation);

	////Set vertex data
	//glBindBuffer(GL_ARRAY_BUFFER, gVBO);
	//glVertexAttribPointer(gVertexPos2DLocation, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), NULL);

	////Set index data and render
	//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gIBO);
	//glDrawElements(GL_TRIANGLE_FAN, 4, GL_UNSIGNED_INT, NULL);

	////Disable vertex position
	//glDisableVertexAttribArray(gVertexPos2DLocation);

	//Unbind program
	glUseProgram(NULL);

	SDL_GL_SwapWindow(window);

#else
	ttt.start();
	gPixelBuffer->Clear();
	for (int x = 0; x < VIEWPORT_X; x++)
	{
		for (int y = 0; y < VIEWPORT_Y; y++)
		{
			int r = rand() % 255;
			int g = rand() % 255;
			int b = rand() % 255;

			gPixelBuffer->SetPixel(x, y, r, g, b);
		}
	}
	ttt.stop();

	//SDL_RenderClear(renderer);
	gPixelBuffer->Draw();
	SDL_RenderPresent(renderer);
#endif

	while (SDL_PollEvent(&sdlEvent))
	{
		switch (sdlEvent.type)
		{
		case SDL_QUIT:
			running = false;
			break;
		case SDL_KEYDOWN:
			gInput.SetKey(sdlEvent, true);
			break;
		case SDL_KEYUP:
			gInput.SetKey(sdlEvent, false);
			break;
		}
	}
}
//-----------------------------------------------------------------------------
int main([[maybe_unused]] int argc, [[maybe_unused]] char* argv[])
{
	PrintLog("SDL Init");
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) != 0)
		return 0;

#if USE_OPENGL
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

	window = SDL_CreateWindow("test", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, RESOLUTION_X, RESOLUTION_Y, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_SHOWN);
	if (!window) return 0;

	gContext = SDL_GL_CreateContext(window);

	gladLoadGL((GLADloadfunc)SDL_GL_GetProcAddress);
	if (SDL_GL_SetSwapInterval(1) < 0)
	{
		printf("Warning: Unable to set VSync! SDL Error: %s\n", SDL_GetError());
	}

	initGL();

	glClearColor(0.4f, 0.6f, 1.0f, 1.0f);
	//glViewport(0, 0, WindowWidth, WindowHeight);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
#else
	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "0");
	//SDL_SetHint(SDL_HINT_RENDER_BATCHING, "1");

	PrintLog("SDL create window");
	window = SDL_CreateWindow("test", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, RESOLUTION_X, RESOLUTION_Y, SDL_WINDOW_RESIZABLE | SDL_WINDOW_SHOWN);
	if (!window) return 0;

	PrintLog("SDL create renderer");
	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
	if (!renderer) return 0;

	//SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND); // TODO: ????
	SDL_RenderSetLogicalSize(renderer, VIEWPORT_X, VIEWPORT_Y);
#endif

	gPixelBuffer = new PixelBuffer<VIEWPORT_X, VIEWPORT_Y>(renderer);

	startTime = std::chrono::high_resolution_clock::now();
#if defined(__EMSCRIPTEN__)
	emscripten_set_main_loop(UpdateFrame, 0, 1);
#else
	while (running) UpdateFrame();
#endif

	delete gPixelBuffer;
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
	return 0;
}
//-----------------------------------------------------------------------------