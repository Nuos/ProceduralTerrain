#ifndef __MESH_LIST_H_
#define __MESH_LIST_H_
#include <iostream>
#include "Mesh.hpp"

class MeshList {
public:
	MeshList::MeshList();
	MeshList::~MeshList();

	//copy constructor
	MeshList::MeshList(const MeshList& other);

	//copy assignment operator
	MeshList& MeshList::operator=(const MeshList& rhs);

	//array subscript operator
	Mesh& MeshList::operator[](int i);

	//modify list
	void MeshList::push_back(Mesh& mesh);
	void MeshList::pop_back();
	void MeshList::resize(int n);
	void MeshList::resize(int n, Mesh val);
	void MeshList::remove(int index);

	void MeshList::loadFile(const char* fileName);
	

	void MeshList::bindAllVertexArrays();

	//get methods
	int MeshList::size();

	friend glm::mat4 aiToMat4(aiMatrix4x4 matrix);
	

private:
	void MeshList::loadNodeData(aiNode *node);
	aiNode *root;
	vector<Mesh> meshList;
	const aiScene * scene;
};

#endif