#include "ShaderProgram.hpp"

namespace shm {
	ShaderProgram::ShaderProgram(){
		shaders.resize(4);
	}

	ShaderProgram::ShaderProgram(Shader vert, Shader frag){
		shaders.resize(4);
		shaders[VERT] = vert;
		shaders[FRAG] = frag;
	}

	ShaderProgram::ShaderProgram(Shader vert, Shader geo, Shader tess, Shader frag){
		shaders.resize(4);
		shaders[VERT] = vert;
		shaders[GEO] = geo;
		shaders[TESS] = tess;
		shaders[FRAG] = frag;
	}

	ShaderProgram::ShaderProgram(Shader shaders[], int count){
		for(int ii = 0; ii < count; ii++){
			switch(shaders[ii].getType()) {
			case GL_VERTEX_SHADER:
				shaders[VERT] = shaders[ii];
				break;
			case GL_GEOMETRY_SHADER:
				shaders[GEO] = shaders[ii];
				break;
			case GL_TESS_CONTROL_SHADER:
				shaders[TESS] = shaders[ii];
				break;
			case GL_FRAGMENT_SHADER:
				shaders[FRAG] = shaders[ii];
				break;
			default:
				cout << "ERROR @ShaderProgram: Array subscript out of bounds" << endl;
				event_log << "ERROR @ShaderProgram: Array subscript out of bounds" << endl;
				exit(-1);
				break;
			}
		}
	}


	ShaderProgram::~ShaderProgram(){}

	ShaderProgram::ShaderProgram(const ShaderProgram& other){
		*this = other;
	}

	ShaderProgram& ShaderProgram::operator=(const ShaderProgram& other){
		for(int j = 0; j < 4; j++){
			shaders[j] = other.shaders[j];
		}
		pHandle = other.pHandle;
		return *this;
	}

	//using the ENUM ShaderStage is recommended
	Shader& ShaderProgram::operator[](int index){
		switch(index) {
		case VERT:
			return shaders[VERT];
			break;
		case GEO:
			return shaders[GEO];
			break;
		case TESS:
			return shaders[TESS];
			break;
		case FRAG:
			return shaders[FRAG];
			break;
		default:
			cout << "ERROR @ShaderProgram: Array subscript out of bounds" << endl;
			event_log << "ERROR @ShaderProgram: Array subscript out of bounds" << endl;
			exit(-1);
			break;
		}
	}

	void ShaderProgram::setVert(Shader vertex_shader){
		shaders[VERT] = vertex_shader;	
	}
	void ShaderProgram::setGeo(Shader geometry_shader){
		shaders[GEO]= geometry_shader;
	}
	void ShaderProgram::setTess(Shader tessellation_shader){
		shaders[TESS] = tessellation_shader;
	}
	void ShaderProgram::setFrag(Shader fragment_shader){
		shaders[FRAG] = fragment_shader;
	}

	void ShaderProgram::compileShaders(){
		for(int i = 0; i < 4; i++){
			if(shaders[i].isLoaded())
				shaders[i].compileShader();
		}
	}

	//creates a program, attaches shaders to it, and links the program
	//INPUT:  a vector of shaders that consist of at least one vertex
	//		  shader and one fragment shader
	//OUTPUT: handle to the shader program
	void ShaderProgram::linkShaders() {
		try{
			if(!shaders[VERT].isLoaded() || !shaders[FRAG].isLoaded()){
				throw "LINK ERROR: At least 2 shaders (vert and frag) required to link";
			}
		} catch (const char *ss) {
			cout << ss << endl;
			event_log << ss << endl;
			exit(-1);
		}
		pHandle = glCreateProgram();

		glAttachShader(pHandle, shaders[VERT].getHandle());
		if(shaders[GEO].isLoaded())
			glAttachShader(pHandle, shaders[GEO].getHandle());
		if(shaders[TESS].isLoaded()){
			glAttachShader(pHandle, shaders[TESS].getHandle());
		}
		glAttachShader(pHandle, shaders[FRAG].getHandle());

		glLinkProgram(pHandle);
		checkShaderLinkError(pHandle);
	}

	GLuint ShaderProgram::getHandle(){
		return pHandle;
	}

	//Check for Linking errors
	//INPUT: index of linked shader program
	void checkShaderLinkError(const GLuint &program){
		GLint params = -1;
		try{
			glGetProgramiv(program, GL_LINK_STATUS, &params);
			if(GL_TRUE != params) {
				throw params;
			} else {
				printProgramInfoLog(program);	
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

	int ShaderProgram::getNumShaders(){
		return shaders.size();
	}
}