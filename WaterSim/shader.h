#ifndef SHADER_H
#define SHADER_H

#include <glad/glad.h>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <cerrno>

std::string get_file_contents(const char* filename);

class Shader 
{

	public:
		GLuint ID;
		Shader(const char* vertexFile, const char* fragmentFile);
		Shader() = default;

		void Activate();
		void Delete();
};

#endif