#include "../include/shader_utils.h"

#include <SDL.h> // SDL2 for base window and OpenGL context init.
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp> // access to glm::value_ptr

#include <cstdlib>
#include <cstddef>
#include <cmath>
#include <iostream>

using std::cerr;
using std::endl;

// Anon namespace for internal linkage.
namespace {

// Constants.
const char * const CUBE_VERTEX_SHADER = "glsl/cube.v.glsl";
const char * const CUBE_FRAGMENT_SHADER = "glsl/cube.f.glsl";

// GLSL program handle
GLuint program;
// Cube vertices buffer handles.
GLuint vbo_cube_vertices, vbo_cube_colors;
// Input variables for the vertex shader.
GLint attribute_coord3d, attribute_v_color;
// IBO handle.
GLuint ibo_cube_elements;
// Uniform used to pass MVP matrix.
GLint uniform_mvp;
// Define the aspect ratio.
int screen_width = 800, screen_height = 600;

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
	GLfloat cube_vertices[] = {
		// front of the cube.
		-1.0, -1.0, 1.0,
		1.0, -1.0, 1.0,
		1.0, 1.0, 1.0,
		-1.0, 1.0, 1.0,
		// back of the cube.
		-1.0, -1.0, -1.0,
		1.0, -1.0, -1.0,
		1.0, 1.0, -1.0,
		-1.0, 1.0, -1.0
	};
	glGenBuffers(1, &vbo_cube_vertices);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_cube_vertices);
	glBufferData(GL_ARRAY_BUFFER,
			sizeof(cube_vertices),
			cube_vertices,
			GL_STATIC_DRAW);

	GLfloat cube_colors[] = {
		// front colors of the cube.
		1.0, 0.0, 0.0,
		0.0, 1.0, 0.0,
		0.0, 0.0, 1.0,
		1.0, 1.0, 1.0,
		// back colors of the cube.
		1.0, 0.0, 0.0,
		0.0, 1.0, 0.0,
		0.0, 0.0, 1.0,
		1.0, 1.0, 1.0
	};
	glGenBuffers(1, &vbo_cube_colors);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_cube_colors);
	glBufferData(GL_ARRAY_BUFFER,
			sizeof(cube_colors),
			cube_colors,
			GL_STATIC_DRAW);

	// Specify the triangles using the index of the vertices in the array.
	GLushort cube_elements[] = {
		// front
		0, 1, 2,
		2, 3, 0,
		// top
		1, 5, 6,
		6, 2, 1,
		// back
		7, 6, 5,
		5, 4, 7,
		// bottom,
		4, 0, 3,
		3, 7, 4,
		// left
		4, 5, 1,
		1, 0, 4,
		// right
		3, 2, 6,
		6, 7, 3
	};

	glGenBuffers(1, &ibo_cube_elements);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo_cube_elements);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER,
			sizeof(cube_elements),
			cube_elements,
			GL_STATIC_DRAW);

	GLuint vs, fs;
	if ((vs = create_shader(CUBE_VERTEX_SHADER, GL_VERTEX_SHADER)) == 0)
		return false;

	if ((fs = create_shader(CUBE_FRAGMENT_SHADER,
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

	// Bind attribute names for the GLSL program
	// NOTE: all of the names should be global constants in this case..
	const char *attribute_name = "coord3d";
	attribute_coord3d = glGetAttribLocation(program, attribute_name);
	if (attribute_coord3d == -1) {
		cerr << "Could not bind attribute " << attribute_name << endl;
		return false;
	}

	attribute_name = "v_color";
	attribute_v_color = glGetAttribLocation(program, attribute_name);
	if (attribute_v_color == -1) {
		cerr << "Could not bind attribute " << attribute_name << endl;
		return false;
	}

	const char *uniform_name = "mvp";
	uniform_mvp = glGetUniformLocation(program, uniform_name);
	if (uniform_mvp == -1) {
		cerr << "Could not bind uniform " << uniform_name << endl;
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
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Tell it to use the GLSL program that we made.
	glUseProgram(program);

	// Pass all of the triangle information into the GLSL program.
	glEnableVertexAttribArray(attribute_coord3d);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_cube_vertices);
	glVertexAttribPointer(attribute_coord3d, // attribute
				3, // number of elements for the input.
				GL_FLOAT, // type of each element.
				GL_FALSE, // take the values as-is.
				0, // stride.
				0); // offset of the first element.

	glEnableVertexAttribArray(attribute_v_color);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_cube_colors);
	glVertexAttribPointer(attribute_v_color, // attribute
				3, // number of elements for the input.
				GL_FLOAT,
				GL_FALSE,
				0,
				0); // offset.

	// Give denoting which vertices make the triangles that are to be drawn.
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo_cube_elements);
	int size;
	glGetBufferParameteriv(GL_ELEMENT_ARRAY_BUFFER, GL_BUFFER_SIZE, &size);
	glDrawElements(GL_TRIANGLES,
			size/sizeof(GLushort),
			GL_UNSIGNED_SHORT,
			0);

	glDisableVertexAttribArray(attribute_coord3d);
	glDisableVertexAttribArray(attribute_v_color);

	// Display the result.
	SDL_GL_SwapWindow(window);
}

//
// Free all resources that were being used by the library.
//
void free_resources()
{
	glDeleteProgram(program);
	glDeleteBuffers(1, &vbo_cube_vertices);
	glDeleteBuffers(1, &vbo_cube_colors);
	glDeleteBuffers(1, &ibo_cube_elements);
}

//
// Have the triangle rotate and translate in oscillation.
//
void input_logic()
{
	// Create the MVP matrix
	// Model: Going to world coordinates, pushing the cube back.
	glm::mat4 model = glm::translate(glm::mat4(1.0f),
						glm::vec3(0.0, 0.0, -4.0));

	// View: Positioning the camera. (A little up and facing straight)
	glm::mat4 view = glm::lookAt(glm::vec3(0.0, 2.0, 0.0),
					glm::vec3(0.0, 0.0, -4.0),
					glm::vec3(0.0, 1.0, 0.0));

	// Projection: Project into the camera plane.
	glm::mat4 projection = glm::perspective(45.0f,
						1.0f*screen_width/screen_height,
						0.1f,
						10.0f);

	// Create the matrix for the animation this frame.
	float angle = SDL_GetTicks() / 1000.0 * 45; // 45 degree per second.
	glm::vec3 axis_y(0, 1, 0);
	glm::mat4 anim = glm::rotate(glm::mat4(1.0f),
					glm::radians(angle),
					axis_y);

	glm::mat4 mvp = projection * view * model * anim;

	glUniformMatrix4fv(uniform_mvp, 1, GL_FALSE, glm::value_ptr(mvp));
	glUseProgram(program);
}

//
// Change the size of the viewport.
//
void on_resize(int width, int height)
{
	screen_width = width;
	screen_height = height;	
	glViewport(0, 0, screen_width, screen_height);
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

			// Check if there was a size change of the window.
			if (ev.type == SDL_WINDOWEVENT &&
				ev.window.event ==
					SDL_WINDOWEVENT_SIZE_CHANGED) {

				on_resize(ev.window.data1, ev.window.data2);
			}
		}

		input_logic();
		render(window);
	}
}

// End of anon namespace.
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
						screen_width,
						screen_height,
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

	glEnable(GL_DEPTH_TEST);

	// If everything has gone okay, we can display something.
	main_loop(window);

	// If the program exits in the usual way, free resources
	// and exit success.
	free_resources();

	return EXIT_SUCCESS;
}
