#ifndef __MESH_LOADER_H_
#define __MESH_LOADER_H_
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <GL/glew.h>
#include <vector>
#include <glm/glm.hpp>
#include <SOIL.h>

using namespace std;

enum bufferDataType {
	VBO_VERTICES,
	VBO_TEXTURES,
	VBO_COLORS,
	VBO_NORMALS,
	VBO_INDEX,
	VBO_UNIFORM
};

enum drawMode {
	DRAW_ORDERED,
	DRAW_INDEXED
};

struct VBO {
	VBO::VBO(bufferDataType type)
		: dataType(type), index(0), attribPointerIndex(0), pointsPerVertex(0),
		buffer_target(GL_ARRAY_BUFFER){}
	bufferDataType dataType;
	GLuint index, attribPointerIndex;
	GLint pointsPerVertex;
	GLenum buffer_target;
};



//Mesh Class:
//	Mesh instance == one VAO
//This class is structured so that every mesh instance 
//has exactly one VAO associated with it. Also keeps track 
//of VBOs attached to said VAO.
class Mesh {
public:
	Mesh::Mesh();
	Mesh::~Mesh();

	//Mesh::Mesh(const DisplaceTerrain& dt);

	//copy constructor
	Mesh::Mesh(const Mesh& other);

	//copy assignment operator
	Mesh& Mesh::operator=(const Mesh& rhs);

	friend class MeshList;

	//Updates mesh with new values
	//INPUT:	Mesh object
	virtual void Mesh::updateMesh(Mesh& m);
	
	//**************************
	//	   VBO & VAO stuff
	//**************************

	//Creates a buffer
	//INPUT:	The type of data that will be in the buffer
	//			Types are: VBO_VERTICES, VBO_TEXTURES, VBO_NORMALS
	VBO Mesh::createBuffer(bufferDataType type);


	//Creates buffers for all valid buffer types
	void Mesh::createAllBuffers();

	//creates uniform buffer block containing material properties
	//NOTE:	requries a linked program to work
	void Mesh::createUniformBlock(GLuint program, const char* blockName, const char** varNames);


	//Manually binds a buffer as the current vertex buffer
	//This is to be used in the main rendering loop to re-bind buffers
	//INPUT:	a VBO object
	void Mesh::bindBuffer(VBO buffer);


	//Binds all available buffers
	//This is to be used in the main rendering loop to re-bind buffers
	void Mesh::bindAllBuffers();

	//Separately loads texture for mesh
	virtual void Mesh::loadTexture(const char *filename, GLenum type);


	//Create VAO, bind buffers to VAO,
	//and enableVertexAttribArray() for each buffer object
	void Mesh::createVertexArray();


	//Binds this mesh's VAO
	//and also binds all buffers associated with it
	void Mesh::bindVertexArray();

	//Updates the vertex array with new data
	void Mesh::updateVertexArray();


	//Enables a buffer in array
	//This method binds VAO before calling 
	//glEnableVertexAttribArray()
	void Mesh::enableVertexAttribArray(VBO buffer);
	void Mesh::enableVertexAttribArray(bufferDataType type);
	void Mesh::enableAllVertexAttribArrays();


	//Disables a buffer in array
	//This method binds VAO before calling 
	//glDisableVertexAttribArray()
	void Mesh::disableVertexAttribArray(VBO buffer);
	void Mesh::disableVertexAttribArray(bufferDataType type);
	void Mesh::disableAllVertexAttribArrays();


	//****************************
	//		SET Functions
	//****************************
	void Mesh::applyModelMatrix(const glm::mat4& modelmat);
	void Mesh::updateModelMatrix();
	void Mesh::setModelMatrix(glm::mat4 &m);
	void Mesh::setAmbientColor(float r, float g, float b);
	void Mesh::setAmbientColor(float val);
	void Mesh::setDiffuseColor(float r, float g, float b);
	void Mesh::setDiffuseColor(float val);
	void Mesh::setSpecularColor(float r, float g, float b);
	void Mesh::setSpecularColor(float val);
	void Mesh::setAsLightSource();
	void Mesh::unsetLightSource();

	//manually defining vertices
	void Mesh::setVertices(float *verts, int size);
	void Mesh::setVertices(vector<float> verts);
	void Mesh::setNormals(float *norms, int size);
	void Mesh::setNormals(vector<float> norms);
	void Mesh::setVertIndicies(GLubyte *index, int size);
	void Mesh::setVertIndicies(vector<GLubyte> index);


	//****************************
	//		Get Functions
	//****************************
	float *Mesh::getVertices();
	float *Mesh::getTextures();
	float *Mesh::getNormals();
	GLubyte *Mesh::getIndices();
	unsigned int Mesh::getNumVertices();
	float * Mesh::getModelMatrix();
	glm::mat4 Mesh::getModelMat4();
	float * Mesh::getAmbientReflectance();
	float * Mesh::getDiffuseReflectance();
	float * Mesh::getSpecularReflectance();
	float Mesh::isLightSource();
	float Mesh::hasTexture();

protected:

	
	

	drawMode draw_mode;
	vector<float> g_vp, g_vt, g_vn; //vertices, textures, and normals
	vector<GLubyte> g_vi;

	glm::mat4 apply_model_mat;
	glm::mat4 baseModelMatrix;
	glm::mat4 modelMatrix;

	GLuint vao, tex;
	vector<VBO> vboList;

	unsigned int g_vert_count;
	
	aiNode * parentNode;
	bool meshLoaded;

	float textureLoaded;
	float is_light_source;
	int attrib_pointer_count;

	//material properties
	glm::vec3 ambient_reflectance,
			  diffuse_reflectance,
			  spec_reflectance;

	//Uniform buffer object attributes
	GLuint ubo, uboIndex;
	GLubyte * blockBuffer;
	enum{Ka, Kd, Ks, LightSource, Texture, numUniforms};
	//GLchar **varNames;
	GLuint blockIndices[numUniforms];
	GLint uboSize,
		  blockSize[numUniforms],
		  blockOffset[numUniforms],
		  blockType[numUniforms];
};

#endif