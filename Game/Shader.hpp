#ifndef __SHADER_CLASS_H_
#define __SHADER_CLASS_H_
#include <string>
#include <iostream>
#include <fstream>
#include <vector>
#include <GL/glew.h>
//#include <GL/glext.h>
#include "event_logger.h"

using namespace std;

namespace CustomShader{
	class Shader {

	public:	

		Shader::Shader();
		Shader::Shader(GLenum type);
		Shader::~Shader();
		void Shader::setType(GLenum type);

		bool Shader::loadFile(const char *fileName);
		bool Shader::compileShader();
		bool Shader::isLoaded();

		//enum BufferType { VBO, VAO };

		////********************************
		////	Vertex Buffer/Array Objects
		////********************************
		//void Shader::createBuffer(GLuint &index, BufferType type);
		//void Shader::bindBuffer(GLuint &index);
		//void Shader::bindVertexArray(GLuint &index);

		//string Shader::getShader();
		GLuint Shader::getHandle();


		//****************Shader Error checking*****************************
		friend void checkShaderCompileError(const GLuint &shader_index);
		friend void printShaderInfoLog(const GLuint &shader_index);


	private:
		GLenum type;
		bool typeInit, shaderLoaded;
		GLuint handle;
		string shaderStr;
		const char * str;
		char buffer[512];
	};
	
}

#endif