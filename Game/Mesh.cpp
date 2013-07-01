#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vector>
#include <iostream>
#include <windows.h>
#include "Mesh.hpp"
#include "event_logger.h"

//PFNGLACTIVETEXTUREPROC glActiveTexture = (PFNGLACTIVETEXTUREPROC) wglGetProcAddress ("glActiveTexture");

using namespace std;

//**********************************************************
//						Mesh Class
//**********************************************************

//Constructor
Mesh::Mesh() : g_vert_count(0), attrib_pointer_count(0), meshLoaded(false),
			   textureLoaded(0.0f), is_light_source(0.0f), modelMatrix(glm::mat4(1.0)), 
			   parentNode(NULL), diffuse_reflectance(glm::vec3(0.5)), 
			   apply_model_mat(glm::mat4(1.0)), baseModelMatrix(glm::mat4(1.0)),
			   draw_mode(DRAW_ORDERED){
}

Mesh::~Mesh(){ 

}

//copy constructor
Mesh::Mesh(const Mesh& other){
	*this = other;
}

//copy assignment operator
Mesh& Mesh::operator=(const Mesh& rhs){
	if(this != &rhs){
		vao = rhs.vao;
		attrib_pointer_count = rhs.attrib_pointer_count;
		g_vert_count = rhs.g_vert_count;
		g_vp = rhs.g_vp;
		g_vt = rhs.g_vt;
		g_vn = rhs.g_vn;
		meshLoaded = rhs.meshLoaded;
		textureLoaded = rhs.textureLoaded;
		is_light_source = rhs.is_light_source;
		vboList = rhs.vboList;
		modelMatrix = rhs.modelMatrix;
		baseModelMatrix = rhs.baseModelMatrix;
		apply_model_mat = rhs.apply_model_mat;
	}
	return *this;
}

//Updates the terrain mesh with new values
//INPUT:	Mesh object
void Mesh::updateMesh(Mesh& m){

	*this = m;
}

//Creates a buffer
//INPUT:	The type of data that will be in the buffer
//			Types are: VBO_VERTICES, VBO_TEXTURES, VBO_NORMALS
VBO Mesh::createBuffer(bufferDataType type){
	if(!meshLoaded){
		cout << "ERROR: Mesh not loaded!" << endl;
		event_log << "ERROR: Mesh not loaded!" << endl;
		exit(-1);
	}
	VBO vbo(type);
	glGenBuffers(1, &vbo.index);
	glBindBuffer(GL_ARRAY_BUFFER, vbo.index);
	switch(vbo.dataType) {
	case VBO_VERTICES:
		if(g_vp.size() > 0) {
			glBufferData(GL_ARRAY_BUFFER, g_vert_count*3*sizeof(float),&g_vp[0],GL_STATIC_DRAW);
			vbo.pointsPerVertex = 3;
			//event_log << "Vertex buffer " << vbo.index << " created" << endl;
		}
		else {
			//no vertices in mesh, cant create buffer
			//this is an extra check; if there were no vertices, program
			//should have complained about it by now
			cout << "BUFFER ERROR: no vertices in mesh, cant create buffer" << endl;
			event_log << "BUFFER ERROR: no vertices in mesh, cant create buffer" << endl;
			exit(-1);
		}
		break;
	case VBO_TEXTURES:
		if(g_vt.size() > 0){
			glBufferData(GL_ARRAY_BUFFER, g_vert_count*2*sizeof(float),&g_vt[0],GL_STATIC_DRAW);
			vbo.pointsPerVertex = 2;
			//event_log << "Texture buffer " << vbo.index << " created" << endl;
			textureLoaded = 1.0f;
		}
		else{
			//no textures in mesh, cant create buffer
			cout << "BUFFER ERROR: no textures in mesh, cant create buffer" << endl;
			event_log << "BUFFER ERROR: no textures in mesh, cant create buffer" << endl;
			exit(-1);
		}
		break;
	case VBO_NORMALS:
		if(g_vn.size() > 0){
			glBufferData(GL_ARRAY_BUFFER, g_vert_count*3*sizeof(float),&g_vn[0],GL_STATIC_DRAW);
			vbo.pointsPerVertex = 3;
			//event_log << "Normals buffer " << vbo.index << " created" << endl;
		} 
		else {
			//no normals in mesh, cant create buffer
			cout << "BUFFER ERROR: no normals in mesh, cant create buffer" << endl;
			event_log << "BUFFER ERROR: no normals in mesh, cant create buffer" << endl;
			exit(-1);
		}
		break;
	case VBO_INDEX:
		if(g_vi.size() > 0) {
			vbo.buffer_target = GL_ELEMENT_ARRAY_BUFFER;
			glBufferData(vbo.buffer_target, sizeof(g_vi.data()),&g_vi[0],GL_STATIC_DRAW);
			vbo.pointsPerVertex = 3;
		}
		else {
			//no indicies specified, cant create buffer
			//this is an extra check; if there were no indicies, program
			//should have complained about it by now
			cout << "BUFFER ERROR: no indicies specified, cant create buffer" << endl;
			event_log << "BUFFER ERROR: no indicies specified, cant create buffer" << endl;
			exit(-1);
		}
		break;
	default:
		break;
	}
	vbo.attribPointerIndex = attrib_pointer_count++;
	vboList.push_back(vbo);
	return vbo;
}

//Creates buffers for all valid buffer types
//	except uniform buffers, because those must be created
//	after a shader program has been linked
void Mesh::createAllBuffers(){
	//check if we have vertices, textures, normals , etc.
	//and then call createBuffer() for each available one

	if(g_vp.size() > 0)
		createBuffer(VBO_VERTICES);
	if(g_vn.size() > 0)
		createBuffer(VBO_NORMALS);
	if(g_vt.size() > 0)
		createBuffer(VBO_TEXTURES);
	if(g_vi.size() > 0)
		createBuffer(VBO_INDEX);
	if(g_vp.size() == 0  &&  g_vt.size() == 0  &&  g_vn.size() == 0) {
		cout << "BUFFER ERROR: vertices empty, Buffer not created" <<endl;
		event_log << "BUFFER ERROR: vertices empty, Buffer not created" <<endl;
		exit(-1);
	}
}

void Mesh::createUniformBlock(GLuint program, const char* blockName, const char** varNames){
	VBO uboData(VBO_UNIFORM);
	uboData.buffer_target = GL_UNIFORM_BUFFER;
	//uniform block initialization
	uboIndex = glGetUniformBlockIndex(program, blockName);

	glGetActiveUniformBlockiv(program,uboIndex,GL_UNIFORM_BLOCK_DATA_SIZE,&uboSize);
	blockBuffer = (GLubyte*)malloc(uboSize);

	//get uniform indices, then find attributes of each uniform inside the block
	glGetUniformIndices(program,numUniforms,varNames,blockIndices);
	glGetActiveUniformsiv(program,numUniforms, blockIndices,GL_UNIFORM_SIZE,blockSize);
	glGetActiveUniformsiv(program,numUniforms, blockIndices,GL_UNIFORM_OFFSET,blockOffset);
	glGetActiveUniformsiv(program,numUniforms, blockIndices,GL_UNIFORM_TYPE,blockType);


	float isLight = is_light_source;
	float hasTex = textureLoaded;

	memcpy( blockBuffer + blockOffset[Ka], 
		getAmbientReflectance(), blockSize[Ka]*3*sizeof(GLfloat));
	memcpy( blockBuffer + blockOffset[Kd], 
		getDiffuseReflectance(), blockSize[Kd]*3*sizeof(GLfloat));
	memcpy( blockBuffer + blockOffset[Ks], 
		getSpecularReflectance(), blockSize[Ks]*3*sizeof(GLfloat));
	memcpy( blockBuffer + blockOffset[LightSource], 
		&isLight, blockSize[LightSource]*sizeof(GLfloat));
	memcpy( blockBuffer + blockOffset[Texture], 
		&hasTex, blockSize[Texture]*sizeof(GLfloat));

	glGenBuffers(1, &uboData.index);
	glBindBuffer(uboData.buffer_target,uboData.index);
	glBufferData(uboData.buffer_target, uboSize, blockBuffer, GL_DYNAMIC_DRAW);
	glBindBufferBase(uboData.buffer_target, uboIndex, uboData.index);
	vboList.push_back(uboData);
}

//Manually binds a buffer as the current vertex buffer
//This is to be used in the main rendering loop to re-bind buffers
//INPUT:	a VBO object
void Mesh::bindBuffer(VBO buffer){
	if(buffer.dataType == VBO_INDEX){
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer.index);
		//glVertexPointer(3, GL_FLOAT, 0, &g_vp[0]);
		glVertexAttribPointer(buffer.attribPointerIndex, buffer.pointsPerVertex,
			GL_UNSIGNED_BYTE, GL_FALSE, 0, (GLubyte*)NULL);
		return;
	}
	else if(buffer.dataType == VBO_UNIFORM){
			glBindBuffer(buffer.buffer_target,buffer.index);
			glBufferData(buffer.buffer_target, uboSize, blockBuffer, GL_DYNAMIC_DRAW);
			glBindBufferBase(buffer.buffer_target, uboIndex, ubo);
	}
	glBindBuffer(GL_ARRAY_BUFFER,buffer.index);
	glVertexAttribPointer(buffer.attribPointerIndex, buffer.pointsPerVertex,
						  GL_FLOAT, GL_FALSE, 0, (GLubyte*)NULL);
}

//Binds all available buffers
//This is to be used in the main rendering loop to re-bind buffers
void Mesh::bindAllBuffers(){
	try {
		if(vboList.size() == 0){
			throw "ERROR: Buffer list empty";
		}
	}
	catch(const char* error){
		cout << error << endl;
		event_log << error << endl;
		system("pause");
	}

	for(int i = 0; i < vboList.size(); i++){
		bindBuffer(vboList[i]);
	}
}

void Mesh::loadTexture(const char *filename, GLenum type){
	tex = SOIL_load_OGL_texture(filename, SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_INVERT_Y);

	//check for errors
	if(tex == 0){
		cout << "SOIL loading ERROR: " << SOIL_last_result() << endl;
		event_log << "SOIL loading ERROR: " << SOIL_last_result() << endl;
	}
	glewInit();
	//activate texture 0
	if(glActiveTexture == NULL)
		cout <<" bad news"<<endl;
	//string stuff((const char*)glGetString(GL_EXTENSIONS));
	//event_log << stuff<< endl;
	//cout << stuff<< endl;
	glActiveTexture(GL_TEXTURE0);
	switch(type){
	case GL_TEXTURE_1D:
		glBindTexture(GL_TEXTURE_1D, tex);
		break;
	case GL_TEXTURE_2D:
		glBindTexture(GL_TEXTURE_2D, tex);
		break;
	case GL_TEXTURE_3D:
		glBindTexture(GL_TEXTURE_3D, tex);
		break;
	default:
		break;
	}
	textureLoaded = 1.0f;
}


//Create VAO, bind buffers to VAO,
//and enableVertexAttribArray() for each buffer object
void Mesh::createVertexArray(){
	if(!meshLoaded){
		cout << "ERROR: Mesh not loaded!" << endl;
		event_log << "ERROR: Mesh not loaded!" << endl;
		exit(-1);
	}
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	bindAllBuffers();
	for(int i = 0; i < vboList.size(); i++){
		enableVertexAttribArray(vboList[i]);
	}
}

//Binds this mesh's VAO
//and also binds all buffers associated with it
void Mesh::bindVertexArray(){
	//bind vertex array first
	glBindVertexArray(vao);
	bindAllBuffers();
}

//Updates the vertex array with new data
void Mesh::updateVertexArray(){
	glBindVertexArray(vao);

	for(int i = 0; i < vboList.size(); i++){
		glBindBuffer(vboList[i].buffer_target, vboList[i].index);
		if(textureLoaded > 0.5){
			glBindTexture(GL_TEXTURE_2D, tex);
		} else {
			glBindTexture(GL_TEXTURE_2D, 0);
		}

		if(vboList[i].dataType == VBO_VERTICES){
			if(g_vert_count == 0)
				glBufferData(vboList[i].buffer_target, 0, 0,GL_STATIC_DRAW);
			else
				glBufferData(vboList[i].buffer_target, g_vert_count*3*sizeof(float),&g_vp[0],GL_STATIC_DRAW);
		}

		else if(vboList[i].dataType == VBO_TEXTURES){
			if(g_vert_count == 0)
				glBufferData(vboList[i].buffer_target, 0, 0,GL_STATIC_DRAW);
			else
				glBufferData(vboList[i].buffer_target, g_vert_count*2*sizeof(float),&g_vt[0],GL_STATIC_DRAW);
		}

		else if(vboList[i].dataType == VBO_NORMALS){
			if(g_vert_count == 0)
				glBufferData(vboList[i].buffer_target, 0, 0,GL_STATIC_DRAW);
			else
				glBufferData(vboList[i].buffer_target, g_vert_count*3*sizeof(float),&g_vn[0],GL_STATIC_DRAW);
		}

		else if(vboList[i].dataType == VBO_INDEX){
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, g_vert_count*sizeof(GLubyte), g_vi.data(), GL_STATIC_DRAW);
			glVertexAttribPointer(vboList[i].attribPointerIndex, vboList[i].pointsPerVertex,
				GL_UNSIGNED_BYTE, GL_FALSE, 0, (GLubyte*)NULL);
			return;
		}
		else if(vboList[i].dataType == VBO_UNIFORM){
			glBufferData(vboList[i].buffer_target, uboSize, blockBuffer, GL_DYNAMIC_DRAW);
			glBindBufferBase(GL_UNIFORM_BUFFER, uboIndex, vboList[i].index);
			return;
		}

		glVertexAttribPointer(vboList[i].attribPointerIndex, vboList[i].pointsPerVertex,
						  GL_FLOAT, GL_FALSE, 0, (GLubyte*)NULL);
	}
}

//Enables a buffer in array
//This method binds VAO before calling 
//glEnableVertexAttribArray()
void Mesh::enableVertexAttribArray(VBO buffer){
	glBindVertexArray(vao);
	glEnableVertexAttribArray(buffer.attribPointerIndex);
}
void Mesh::enableVertexAttribArray(bufferDataType type){
	int i;
	for(i = 0; i < vboList.size(); i++){
		if(vboList[i].dataType == type)
			break;
	}
	enableVertexAttribArray(vboList[i]);
}

void Mesh::enableAllVertexAttribArrays(){
	for(int n = 0; n < vboList.size(); n++){
		enableVertexAttribArray(vboList[n]);
	}
}

//Disables a buffer in array
//This method binds VAO before calling 
//glDisableVertexAttribArray()
void Mesh::disableVertexAttribArray(VBO buffer){
	glBindVertexArray(vao);
	glDisableVertexAttribArray(buffer.attribPointerIndex);
}
void Mesh::disableVertexAttribArray(bufferDataType type){
	int i;
	for(i = 0; i < vboList.size(); i++){
		if(vboList[i].dataType == type)
			break;
	}
	disableVertexAttribArray(vboList[i]);
}

void Mesh::disableAllVertexAttribArrays(){
	for(int n = 0; n < vboList.size(); n++){
		disableVertexAttribArray(vboList[n]);
	}
}



//****************************
//		SET Functions
//****************************

void Mesh::applyModelMatrix(const glm::mat4& modelmat)	{ apply_model_mat = modelmat * apply_model_mat; }

void Mesh::updateModelMatrix()	{	modelMatrix = apply_model_mat * baseModelMatrix; }

void Mesh::setModelMatrix(glm::mat4 &m)	{modelMatrix = m;}

void Mesh::setAmbientColor(float r, float g, float b)	{ ambient_reflectance = glm::vec3(r, g, b); }
void Mesh::setAmbientColor(float val)					{ ambient_reflectance = glm::vec3(val); }
void Mesh::setDiffuseColor(float r, float g, float b)	{ diffuse_reflectance = glm::vec3(r, g, b); }
void Mesh::setDiffuseColor(float val)					{ diffuse_reflectance = glm::vec3(val); }
void Mesh::setSpecularColor(float r, float g, float b)	{ spec_reflectance = glm::vec3(r, g, b); }
void Mesh::setSpecularColor(float val)					{ spec_reflectance = glm::vec3(val); }

void Mesh::setAsLightSource()	{ is_light_source = 1.0; }
void Mesh::unsetLightSource()	{ is_light_source = 0.0; }

void Mesh::setVertices(float *verts, int size){
	g_vp.resize(size);

	for(int i = 0; i < size; i++){
		g_vp[i] = verts[i];

	}
	meshLoaded = true;
}

void Mesh::setVertices(vector<float> verts){
	//g_vert_count = verts.size();
	g_vp = verts;
	meshLoaded = true;
}

void Mesh::setNormals(float *norms, int size){
	//g_vert_count = size;
	g_vn.resize(g_vert_count*3);

	for(int i = 0; i < size; i++){
		g_vn[i*3] = norms[i*3];
		g_vn[i*3+1] = norms[i*3+1];
		g_vn[i*3+2] = norms[i*3+2];
	}
}

void Mesh::setNormals(vector<float> norms){
	//g_vert_count = norms.size();
	g_vn = norms;
}

void Mesh::setVertIndicies(GLubyte *index, int size){
	g_vert_count = size;
	g_vi.resize(size);

	for(int i = 0; i < size; i++){
		g_vi[i] = index[i];

	}
}

void Mesh::setVertIndicies(vector<GLubyte> index){
	//g_vert_count = index.size();
	g_vi = index;
}

//****************************
//		Get Functions
//****************************

float *Mesh::getVertices(){
	return g_vp.data();
}
float *Mesh::getTextures(){
	return g_vt.data();
}
float *Mesh::getNormals(){
	return g_vn.data();
}
GLubyte *Mesh::getIndices(){
	return g_vi.data();
}
unsigned int Mesh::getNumVertices(){
	return g_vert_count; 
}
float * Mesh::getModelMatrix(){
	return glm::value_ptr(modelMatrix);
}
glm::mat4 Mesh::getModelMat4(){
	return modelMatrix;
}
float * Mesh::getAmbientReflectance(){
	return glm::value_ptr(ambient_reflectance);
}
float * Mesh::getDiffuseReflectance(){
	return glm::value_ptr(diffuse_reflectance);
}
float * Mesh::getSpecularReflectance(){
	return glm::value_ptr(spec_reflectance);
}
float Mesh::isLightSource(){
	return is_light_source;
}
float Mesh::hasTexture(){
	return textureLoaded;
}

