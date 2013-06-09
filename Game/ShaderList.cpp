#include "ShaderList.hpp"

namespace CustomShader {
	ShaderList::ShaderList(){
		
	}

	ShaderList::~ShaderList(){
	
	}

	void ShaderList::setVert(Shader vertex_shader){
		this->vert = vertex_shader;	
	}
	void ShaderList::setGeo(Shader geometry_shader){
		this->geo = geometry_shader;
	}
	void ShaderList::setTess(Shader tessellation_shader){
		this->tess = tessellation_shader;
	}
	void ShaderList::setFrag(Shader fragment_shader){
		this->frag = fragment_shader;
	}

	//creates a program, attaches shaders to it, and links the program
	//INPUT:  a vector of shaders that consist of at least one vertex
	//		  shader and one fragment shader
	//OUTPUT: handle to the shader program
	void ShaderList::linkShaders() {
		try{
			if(!vert.isLoaded() || !frag.isLoaded()){
				throw "LINK ERROR: At least 2 shaders (vert and frag) required to link";
			}
		} catch (const char *ss) {
			cout << ss << endl;
			event_log << ss << endl;
			exit(-1);
		}
		pHandle = glCreateProgram();

		glAttachShader(pHandle, vert.getHandle());
		glAttachShader(pHandle, frag.getHandle());

		if(geo.isLoaded())
			glAttachShader(pHandle, geo.getHandle());
		if(tess.isLoaded()){
			glAttachShader(pHandle, tess.getHandle());
		}
		glLinkProgram(pHandle);
		checkShaderLinkError(pHandle);
	}

	GLuint ShaderList::getHandle(){
		return this->pHandle;
	}

	//Check for Linking errors
	//INPUT: index of linked shader program
	void checkShaderLinkError(const GLuint &program){
		GLint params = -1;
		try{
			glGetProgramiv(program, GL_LINK_STATUS, &params);
			if(GL_TRUE != params) {
				throw params;
			}
		}
		catch (GLint e) {
			cout << "LINK ERROR: shader program index " << program << " failed to link"<<endl;
			event_log << "LINK ERROR: shader program index " << program << " failed to link"<<endl;
			printProgramInfoLog(program);		
			exit(-1);
		}
	}

	void printProgramInfoLog(const GLuint &program){
		int max_length = 2056, length = 0;
		char log[2056];
		glGetProgramInfoLog(program, max_length, &length, log);
		cout << log << endl;
		event_log << log << endl;
	}

}