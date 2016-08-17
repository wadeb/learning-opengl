//
// Source implementation file for GLSL utility functions.
// Wade Bonkowski - 6/18/2016
//

#include "../include/shader_utils.h"

#include "SDL.h"
#include <iostream>

using std::cerr;
using std::endl;

// Anon namespace for internal linkage.
namespace {

//
// Read a GLSL file into a c-string.
// Use SDL_RWops for Android asset support.
// NOTE: Make sure to delete[] the returned buffer.
//
char *file_read(const char *filename)
{
	SDL_RWops *rw = SDL_RWFromFile(filename, "rb");
	if (rw == nullptr)
		return nullptr;

	Sint64 res_size = SDL_RWsize(rw);
	char *res = new char[res_size + 1];

	Sint64 nb_read_total = 0, nb_read = 1;
	char *buf = res;
	// Read the file by chunks until its all read.
	while (nb_read_total < res_size && nb_read != 0) {
		nb_read = SDL_RWread(rw, buf, 1, (res_size - nb_read_total));
		nb_read_total += nb_read;
		buf += nb_read;
	}

	SDL_RWclose(rw);
	// If the whole file wasn't read, error out.
	if (nb_read_total != res_size) {
		delete[] res;
		return nullptr;
	}

	res[nb_read_total] = '\0';

	return res;
}

// End of anon namespace.
}

//
// Display compilation errors from the OpenGL shader compiler.
//
void print_log(GLuint object)
{
	GLint log_length = 0;
	if (glIsShader(object)) {
		glGetShaderiv(object, GL_INFO_LOG_LENGTH, &log_length);
	} else if (glIsProgram(object)) {
		glGetProgramiv(object, GL_INFO_LOG_LENGTH, &log_length);
	} else {
		cerr << "printlog: Not a shader nor a program." << endl;
		return;
	}

	char *log = new char[log_length];
	if (glIsShader(object)) {
		glGetShaderInfoLog(object, log_length, nullptr, log);
	} else if (glIsProgram(object)) {
		glGetProgramInfoLog(object, log_length, nullptr, log);
	}

	cerr << log;
	delete[] log;
}

//
// Compile the shader from filename with error handling.
//
GLuint create_shader(const char *filename, GLenum type)
{
	const GLchar *source = file_read(filename);
	if (source == nullptr) {
		cerr << "Error opening " << filename << ": " << SDL_GetError()
			<< endl;

		return 0;
	}

	GLuint res = glCreateShader(type);
	glShaderSource(res, 1, &source, nullptr);

	glCompileShader(res);
	GLint compile_ok = GL_FALSE;
	glGetShaderiv(res, GL_COMPILE_STATUS, &compile_ok);
	if (compile_ok == GL_FALSE) {
		cerr << filename << ":";
		print_log(res);
		glDeleteShader(res);
		return 0;
	}

	delete[] source;

	return res;
}
