#ifndef __SHADER_LIST_H_
#define __SHADER_LIST_H_
#include "Shader.hpp"

namespace CustomShader {
	class ShaderList
	{
	public:
		ShaderList::ShaderList();
		ShaderList::~ShaderList();

		void ShaderList::setVert(Shader vertex_shader);
		void ShaderList::setGeo(Shader geometry_shader);
		void ShaderList::setTess(Shader tessellation_shader);
		void ShaderList::setFrag(Shader fragment_shader);
		void ShaderList::linkShaders();

		GLuint ShaderList::getHandle();

		//****************Shader Error checking*****************************

		friend void checkShaderLinkError(const GLuint &program);
		friend void printProgramInfoLog(const GLuint &program);

	private:
		Shader vert, geo, tess, frag;
		GLuint pHandle;
	};
}

#endif