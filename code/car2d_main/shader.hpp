#pragma once

#define NOMINMAX
#include <GL/gl3w.h>
#include <string>

GLuint compile_shader_from_file(const std::string& filepath, GLenum shaderType);
GLuint compile_shader_from_source(const char* source, GLenum shaderType);
GLuint link_program(GLuint program);