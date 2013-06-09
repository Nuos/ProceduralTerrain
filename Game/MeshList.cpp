#include "MeshList.hpp"
#include "event_logger.h"

using namespace std;

//**********************************************************
//					MeshList Class
//**********************************************************

MeshList::MeshList() : root(NULL){
	//
}

MeshList::~MeshList(){
	//
}

//copy constructor
MeshList::MeshList(const MeshList& other){
	meshList = other.meshList;
}

//copy assignment operator
MeshList& MeshList::operator=(const MeshList& rhs){
	if(this != &rhs){
		meshList.resize(rhs.meshList.size());
		for(int i = 0; i < rhs.meshList.size(); i++){
			meshList[i] = rhs.meshList[i];
		}
	}
	return *this;
}

//array subscript operator
Mesh& MeshList::operator[](int i){
	return meshList[i];
}

void MeshList::push_back(Mesh& mesh){
	meshList.push_back(mesh);
}
void MeshList::pop_back(){
	meshList.pop_back();
}
void MeshList::resize(int n){
	meshList.resize(n);
}
void MeshList::resize(int n, Mesh val){
	meshList.resize(n, val);
}
void MeshList::remove(int index){

}

//Load function
void MeshList::loadFile(const char *fileName){
	Assimp::Importer meshImporter;
	scene = meshImporter.ReadFile(fileName, aiProcess_Triangulate);
	if(!scene){
		cout << "Error!! Could not load mesh file" <<endl;
		event_log << "Error: could not load mesh file" << endl;
		exit(-1);
	}
	cout << "Importing..." << endl;
	cout << "Animations: " << scene->mNumAnimations << endl
		<< "Cameras: " << scene->mNumCameras << endl
		<< "Lights: " << scene->mNumLights << endl
		<< "Materials: " << scene->mNumMaterials << endl
		<< "Meshes: " << scene->mNumMeshes << endl
		<< "Textures: " << scene->mNumTextures<< endl;
	//const aiVector3D *vp, *vt, *vn;

	root = scene->mRootNode;
	loadNodeData(root);

}

void MeshList::loadNodeData(aiNode * node){
	aiNode **children = node->mChildren;

	//if this node has meshes, load meshes
	if(node->mNumMeshes > 0){

		unsigned int * nodeMeshes = node->mMeshes;

		for(int m_i = 0; m_i < node->mNumMeshes; m_i++) {
			Mesh mm;
			aiMesh **mesh = scene->mMeshes;
			

			vector<float> vpoint, vtex, vnorm;
			int g_vert_count = mesh[nodeMeshes[m_i]]->mNumVertices;
			cout << "Importing Mesh " << m_i+1 << endl
				<< "Vertex points: " << g_vert_count <<endl;

			//get vertex points, textures, normals, etc.
			for(int v_i = 0; v_i < mesh[nodeMeshes[m_i]]->mNumVertices; v_i++){
				if(mesh[nodeMeshes[m_i]]->HasPositions()){
					const aiVector3D *vp = &(mesh[nodeMeshes[m_i]]->mVertices[v_i]);
					vpoint.push_back(vp->x);
					vpoint.push_back(vp->y);
					vpoint.push_back(vp->z);
				}
				if(mesh[nodeMeshes[m_i]]->HasTextureCoords(0)){
					const aiVector3D *vt = &(mesh[nodeMeshes[m_i]]->mTextureCoords[0][v_i]);
					vtex.push_back(vt->x);
					vtex.push_back(vt->y);
				}
				if(mesh[nodeMeshes[m_i]]->HasNormals()){
					const aiVector3D *vn = &(mesh[nodeMeshes[m_i]]->mNormals[v_i]);
					vnorm.push_back(vn->x);
					vnorm.push_back(vn->y);
					vnorm.push_back(vn->z);
				}
				if(mesh[nodeMeshes[m_i]]->HasTangentsAndBitangents()){
					//just in case
				}
			}

			//if(node->mName.C_Str() == "d")
			mm.parentNode = node->mParent;
			mm.g_vp = vpoint;
			mm.g_vt = vtex;
			mm.g_vn = vnorm;
			mm.g_vert_count = g_vert_count;
			
			if(vpoint.size() > 0)
				mm.meshLoaded = true;
			else if(vtex.size() > 0)
				mm.textureLoaded = true;
			else {
				mm.meshLoaded = false;
				mm.textureLoaded = false;
			}

			////if this is the root node, set first modelMatrix to identity
			//if(node->mParent == NULL){
			//	mm.modelMatrix = glm::mat4(1.0);
			//}
			//else
			mm.modelMatrix = aiToMat4(mm.parentNode->mTransformation * node->mTransformation);
			meshList.push_back(mm);
		}
	}

	//if this node has children, load those
	if(node->mNumChildren > 0){
		
		for(int n = 0; n < node->mNumChildren; n++){
			loadNodeData(children[n]);
		}
	}

	
	cout << "Importing meshes complete" << endl;
	event_log << "Importing meshes complete" << endl;
}


void MeshList::bindAllVertexArrays(){
	//just call bindVertexArray() for each Mesh
}

int MeshList::size(){
	return meshList.size();
}

glm::mat4 aiToMat4(aiMatrix4x4 matrix){
	//transpose first
	aiMatrix4x4 temp = matrix.Transpose();

	glm::mat4 mat(temp.a1, temp.a2, temp.a3, temp.a4,
				  temp.b1, temp.b2, temp.b3, temp.b4,
				  temp.c1, temp.c2, temp.c3, temp.c4,
				  temp.d1, temp.d2, temp.d3, temp.d4);
	return mat;
}