#ifndef __SHADER_PROGRAM_H_
#define __SHADER_PROGRAM_H_
#include "Shader.hpp"

enum ShaderStage {
	VERT,
	GEO,
	TESS,
	FRAG
};
namespace shm {
	class ShaderProgram
	{
	public:
		ShaderProgram::ShaderProgram();
		ShaderProgram::ShaderProgram(Shader vert, Shader frag);
		ShaderProgram::ShaderProgram(Shader vert, Shader geo, Shader tess, Shader frag);
		ShaderProgram::ShaderProgram(Shader shaders[], int count);
		ShaderProgram::~ShaderProgram();

		ShaderProgram::ShaderProgram(const ShaderProgram& other);

		ShaderProgram& ShaderProgram::operator=(const ShaderProgram& other);

		Shader& ShaderProgram::operator[](int index);

		void ShaderProgram::setVert(Shader vertex_shader);
		void ShaderProgram::setGeo(Shader geometry_shader);
		void ShaderProgram::setTess(Shader tessellation_shader);
		void ShaderProgram::setFrag(Shader fragment_shader);
		void ShaderProgram::compileShaders();
		void ShaderProgram::linkShaders();

		GLuint ShaderProgram::getHandle();

		//****************Shader Error checking*****************************
		friend void checkShaderLinkError(const GLuint &program);
		friend void printProgramInfoLog(const GLuint &program);

		//******* GET methods *******/
		int ShaderProgram::getNumShaders();

	private:
		std::vector<Shader> shaders;
		GLuint pHandle;
	};
}

#endif