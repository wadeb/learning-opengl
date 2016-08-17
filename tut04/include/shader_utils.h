#ifndef CREATE_SHADER
#define CREATE_SHADER

//
// Header file that exposes some utility functions for working wth GLSL.
// Wade Bonkowski - 6/18/2016
//

#include <GL/glew.h>

//
// Print more information regarding GLSL compile errors.
//
void print_log(GLuint object);

//
// Compile the shader from filename with error handling.
//
GLuint create_shader(const char *filename, GLenum type);

#endif // CREATE_SHADER
