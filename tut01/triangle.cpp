#include <GL/glew.h> // glew.h rather than gl.h for declarations.
#include <SDL.h> // SDL2 for base window and OpenGL context init.

#include <cstdlib>
#include <iostream>

using std::cerr;
using std::endl;

// GLSL program handle
GLuint program;

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
	GLint compile_ok = GL_FALSE, link_ok = GL_FALSE;

	// Create the vertex shader.
	GLuint vs = glCreateShader(GL_VERTEX_SHADER);
	// Write the source for the shader in place.
	const char *vs_source =
		"#version 120\n"
		"attribute vec2 coord2d;\n"
		"void main(void) {\n"
		"	gl_Position = vec4(coord2d, 0.0, 1.0);\n"
		"}";

	glShaderSource(vs, 1, &vs_source, NULL);
	glCompileShader(vs);
	glGetShaderiv(vs, GL_COMPILE_STATUS, &compile_ok);
	if (!compile_ok) {
		cerr << "Error in vertex shader" << endl;
		return false;
	}

	// Create the fragment shader.
	GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
	const char *fs_source =
		"#version 120\n"
		"void main(void) {\n"
		"	gl_FragColor[0] = 0.0;\n"
		"	gl_FragColor[1] = 0.0;\n"
		"	gl_FragColor[2] = 1.0;\n"
		"}";

	glShaderSource(fs, 1, &fs_source, NULL);
	glCompileShader(fs);
	glGetShaderiv(fs, GL_COMPILE_STATUS, &compile_ok);
	if (!compile_ok) {
		cerr << "Error in fragment shader" << endl;
		return false;
	}

	// Create the GLSL program by linking the vertex
	// and the fragment shaders.
	program = glCreateProgram();
	glAttachShader(program, vs);
	glAttachShader(program, fs);
	glLinkProgram(program);
	glGetProgramiv(program, GL_LINK_STATUS, &link_ok);
	if (!link_ok) {
		cerr << "Error in glLinkProgram" << endl;
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
	// Make the background white to start.
	glClearColor(1.0, 1.0, 1.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT);

	// Tell it to use the GLSL program that we made.
	glUseProgram(program);
	glEnableVertexAttribArray(attribute_coord2d);

	// Set the points of the triangle that we want to draw.
	GLfloat triangle_vertices[] = {
		0.0, 0.8,
		-0.8, -0.8,
		0.8, -0.8
	};

	// Describe our vertices array to OpenGL (it can't guess the format).
	glVertexAttribPointer(attribute_coord2d, // attribute
				2, // number of elements per vertex.
				GL_FLOAT, // type of each element.
				GL_FALSE, // take the values as-is.
				0, // no extra data between each position.
				triangle_vertices); // pointer to the array.

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

	SDL_GL_CreateContext(window);
	// Extension wrangler initializing.
	GLenum glew_status = glewInit();
	if (glew_status != GLEW_OK) {
		cerr << "Error: glewInit: " << endl;
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
