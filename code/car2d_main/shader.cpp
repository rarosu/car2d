#include "shader.hpp"
#include <string>
#include <stdexcept>
#include <fstream>

GLuint compile_shader_from_file(const std::string& filepath, GLenum shaderType)
{
	GLuint shader = 0;
	std::ifstream file(filepath);
	
	if (!file.is_open())
	{
		throw std::runtime_error(std::string("Failed to open shader file: ") + filepath);
	}

	std::string source;
	while (!file.eof())
	{
		std::string line;
		std::getline(file, line);

		source += line + "\n";
	}

	try
	{
		shader = compile_shader_from_source(source.c_str(), shaderType);
	}
	catch (std::runtime_error& e)
	{
		throw std::runtime_error(std::string("[") + filepath + "] " + e.what());
	}

	return shader;
}

GLuint compile_shader_from_source(const char* source, GLenum shaderType)
{
	GLuint shader = glCreateShader(shaderType);
	glShaderSource(shader, 1, &source, nullptr);
	glCompileShader(shader);

	GLint status;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
	if (status != GL_TRUE)
	{
		GLint logSize;
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logSize);

		std::string log;
		if (logSize > 0)
		{
			int written;
			log.resize(logSize);
			glGetShaderInfoLog(shader, logSize, &written, &log[0]);
		}

		throw std::runtime_error("Failed to compile shader: " + log);
	}

	return shader;
}

GLuint link_program(GLuint program)
{
	glLinkProgram(program);

	GLint status;
	glGetProgramiv(program, GL_LINK_STATUS, &status);
	if (status != GL_TRUE)
	{
		GLint logSize;
		glGetProgramiv(program, GL_INFO_LOG_LENGTH, &logSize);

		std::string log;
		if (logSize > 0)
		{
			int written;
			log.resize(logSize);
			glGetProgramInfoLog(program, logSize, &written, &log[0]);
		}

		throw std::runtime_error("Failed to link program: " + log);
	}

	return program;
}