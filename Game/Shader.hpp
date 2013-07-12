#ifndef __SHADER_CLASS_H_
#define __SHADER_CLASS_H_
#include <string>
#include <iostream>
#include <fstream>
#include <vector>
#include <GL/glew.h>
#include "event_logger.h"

using namespace std;


namespace shm {
	class Shader {

	public:	

		Shader::Shader();
		Shader::Shader(GLenum type);
		Shader::Shader(const Shader& other);
		Shader& Shader::operator=(const Shader& other);
		Shader::~Shader();

		void Shader::setType(GLenum type);

		bool Shader::loadFile(const char *fileName);
		bool Shader::compileShader();
		bool Shader::isLoaded();

		//string Shader::getShader();
		GLuint Shader::getHandle();
		GLenum Shader::getType();


		//****************Shader Error checking*****************************
		friend void checkShaderCompileError(const GLuint &shader_index);
		friend void printShaderInfoLog(const GLuint &shader_index);


	private:
		GLenum type;
		bool typeInit, shaderLoaded;
		GLuint handle;
		string shaderStr;
	};
	
}

#endif