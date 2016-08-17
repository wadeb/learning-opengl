#include "../include/shader_utils.h"

#include <SDL.h> // SDL2 for base window and OpenGL context init.

#include <cstdlib>
#include <iostream>

using std::cerr;
using std::endl;

// Constants.
const char * const TRIANGLE_VERTEX_SHADER = "glsl/triangle.v.glsl";
const char * const TRIANGLE_FRAGMENT_SHADER = "glsl/triangle.f.glsl";

// GLSL program handle
GLuint program;
// Triangle VBO handle.
GLuint vbo_triangle;

// Input variable for the vertex shader.
GLint attribute_coord2d;

//
// Initiate resources.
// NOTE: I probably would have done something like write my own shader
//       class that has the program as a static member variable,
//       like a singleton. Each instance of the class would be either
//       a vertex, fragment, or geometry shader.
//       Or something along those lines.
//       This is likely to change as I learn more about this.
//
bool init_resources()
{
	// Set the triangle vertices and load them to the GPU.
	GLfloat triangle_vertices[] = {
		0.0, 0.8,
		-0.8, -0.8,
		0.8, -0.8
	};

	glGenBuffers(1, &vbo_triangle);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_triangle);
	glBufferData(GL_ARRAY_BUFFER,
			sizeof(triangle_vertices),
			triangle_vertices,
			GL_STATIC_DRAW);

	GLuint vs, fs;
	if ((vs = create_shader(TRIANGLE_VERTEX_SHADER, GL_VERTEX_SHADER)) == 0)
		return false;

	if ((fs = create_shader(TRIANGLE_FRAGMENT_SHADER,
				GL_FRAGMENT_SHADER)) == 0) {

		return false;
	}

	// Create the GLSL program by linking the vertex
	// and the fragment shaders.
	program = glCreateProgram();
	glAttachShader(program, vs);
	glAttachShader(program, fs);
	glLinkProgram(program);
	GLint link_ok = GL_FALSE;
	glGetProgramiv(program, GL_LINK_STATUS, &link_ok);
	if (link_ok == GL_FALSE) {
		cerr << "glLinkProgram: ";
		print_log(program);
		return false;
	}

	// Tell the GLSL program where its input is.
	const char *attribute_name = "coord2d";
	attribute_coord2d = glGetAttribLocation(program, attribute_name);
	if (attribute_coord2d == -1) {
		cerr << "Could not bind attribute " << attribute_name << endl;
		return false;
	}

	return true;
}

//
// Render all in window.
//
void render(SDL_Window *window)
{
	// Enable Alpha
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// Make the background white to start.
	glClearColor(1.0, 1.0, 1.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT);

	// Tell it to use the GLSL program that we made.
	glUseProgram(program);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_triangle);
	glEnableVertexAttribArray(attribute_coord2d);

	// Describe our vertices array to OpenGL (it can't guess the format).
	glVertexAttribPointer(attribute_coord2d, // attribute
				2, // number of elements per vertex.
				GL_FLOAT, // type of each element.
				GL_FALSE, // take the values as-is.
				0, // no extra data between each position.
				0); // pointer to the array.

	// Push each element in triangle_vertics into the vertex shader.
	glDrawArrays(GL_TRIANGLES, 0, 3);
	glDisableVertexAttribArray(attribute_coord2d);

	// Display the result.
	SDL_GL_SwapWindow(window);
}

//
// Free all resources that were being used by the library.
//
void free_resources()
{
	glDeleteProgram(program);
	glDeleteBuffers(1, &vbo_triangle);
}

//
// Main loop that keeps rendering.
//
void main_loop(SDL_Window *window)
{
	while (true) {
		SDL_Event ev;
		while (SDL_PollEvent(&ev)) {
			if (ev.type == SDL_QUIT)
				return;
		}

		render(window);
	}
}

//
// Driver.
//
int main()
{
	// SDL initialization.
	SDL_Init(SDL_INIT_VIDEO);
	// Window initialization.
	SDL_Window *window = SDL_CreateWindow("My First Triangle",
						SDL_WINDOWPOS_CENTERED,
						SDL_WINDOWPOS_CENTERED,
						640,
						480,
						SDL_WINDOW_RESIZABLE |
						SDL_WINDOW_OPENGL);

	// Some SDL error handling.
	if (window == nullptr) {
		cerr << "Error: can't create window: " << SDL_GetError()
			<< endl;

		return EXIT_FAILURE;
	}

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
	SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 1);

	if (SDL_GL_CreateContext(window) == nullptr) {
		cerr << "Error: SDL_GL_CreateContext: " 
			<< SDL_GetError() << endl;

		return EXIT_FAILURE;
	}

	// Extension wrangler initializing.
	GLenum glew_status = glewInit();
	if (glew_status != GLEW_OK) {
		cerr << "Error: glewInit: " << endl;
		return EXIT_FAILURE;
	}

	if (!GLEW_VERSION_2_0) {
		cerr << "Error: your graphics card doesn't support OpenGL 2.0"
			<< endl;

		return EXIT_FAILURE;
	}

	// When all of the init functiona have run without errors,
	// the program can initialize the resources.
	if (!init_resources())
		return EXIT_FAILURE;

	// If everything has gone okay, we can display something.
	main_loop(window);

	// If the program exits in the usual way, free resources
	// and exit success.
	free_resources();

	return EXIT_SUCCESS;
}
