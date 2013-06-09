#include "Shader.hpp"
#include <vector>


namespace CustomShader{


	//****************************
	//Constructors / Destructors
	//****************************

	//Constructor
	Shader::Shader(){
		typeInit = false;
		shaderLoaded = false;
	}

	//Constructor
	//INPUT:	Shader type
	//			e.g. GL_VERTEX_SHADER, GL_FRAGMENT_SHADER
	Shader::Shader(GLenum type){
		this->type = type;
		typeInit = true;
		shaderLoaded = false;
	}

	Shader::~Shader(){
	}


	//**************************
	//		Core Functions
	//**************************

	//DESC:	Loads a shader from a file
	//INPUT:	filename in whatever extension, as long as it's text
	//OUTPUT:	returns false if error in opening file,
	//			otherwise returns true
	bool Shader::loadFile(const char *fileName){
		ifstream input;

		cout << "Loading shader: " << fileName << endl;
		event_log << "Loading shader: "<< fileName << endl;
		input.open(fileName);
		if(input.bad() || input.fail()){
			cout << "Error opening shader file" << endl;
			return false;
		}

		while(!input.eof()){
			input.getline(buffer,512);
			shaderStr.append(string(buffer));
			if(buffer[0] == '#')
				shaderStr.append("\r\n");
			else if(buffer[0] == '/' && buffer[1] == '/')
				shaderStr.append("\r\n");
		}
		cout << "Shader loaded!" << endl;
		event_log << "Shader loaded!" << endl;
		shaderLoaded = true;
		return true;
	}

	//DESC:	Compile loaded shader
	//OUTPUT:	returns false if shader not loaded, or shader type is 
	//				not defined
	bool Shader::compileShader(){
		try {
			if(typeInit){
				handle = glCreateShader(type);
				if(shaderLoaded){
					str = shaderStr.c_str();
					glShaderSource(handle, 1, &str, NULL);
					glCompileShader(handle);
					checkShaderCompileError(handle);
					return true;
				}
				else {
					cout << "error: shader not loaded" << endl;
					event_log << "error: shader not loaded" << endl;
					return false;
				}

			} 
			else {
				throw "SHADER COMPILE ERROR: Shader type not defined";
			}
		} catch ( const char * str) {
			cout << str << endl;
			event_log << str << endl;
			exit(-1);
		}
	}

	//Returns the state of this shader: loaded or not
	bool Shader::isLoaded(){
		return shaderLoaded;
	}


	//*********************
	//		Setters
	//*********************

	//Set shader type
	//e.g. GL_VERTEX_SHADER, GL_FRAGMENT_SHADER
	void Shader::setType(GLenum type){
		this->type = type;
		typeInit = true;
	}


	//*********************
	//		Getters
	//*********************

	//returns shader program as a string
	//string Shader::getShader(){
	//	return shader;
	//}

	//Returns this shader's handle (index)
	GLuint Shader::getHandle(){
		return this->handle;
	}


	//************************
	//		Error Checking
	//************************

	//Check for shader compile errors
	//INPUT:	index of compiled shader
	void checkShaderCompileError(const GLuint &shader_index){
		GLint params = -1;
		try {
			glGetShaderiv(shader_index,GL_COMPILE_STATUS, &params);
			if (GL_TRUE != params) {
				throw params;
			}
		} 
		catch (GLint e){
			cout << "COMPILE ERROR: GL shader index " << shader_index << " did not compile"<<endl;
			event_log << "COMPILE ERROR: GL shader index " << shader_index << " did not compile"<<endl;
			printShaderInfoLog(shader_index);
			exit(-1);
		}
	}

	void printShaderInfoLog(const GLuint &shader_index){
		int max_length = 2056, length = 0;
		char log[2056];
		glGetShaderInfoLog(shader_index, max_length, &length, log);
		cout << log << endl;
		event_log << log << endl;
	}
}