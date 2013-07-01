#include "QuadTree.hpp"
#include "event_logger.h"
#include <iostream>

int QuadTree::totalNodes = 0;
int QuadTree::nodeCount = 0;
int QuadTree::MAX_DEPTH = -1;

std::vector<QuadTree*> QuadTree::selected;

float QuadTree::min_distance = 99999;

//gets the exponent of a power-of-2 number
//int logBase2(int num);

QuadTree::QuadTree(int level, BoundingBox box) 
	: level(level), has_children(false), has_data(false), 
	box(box), n(box.width), LOD_level(0) {
		if(level == 0) {
			MAX_DEPTH = logBase2(n);
			LOD_level = MAX_DEPTH - 1;
		}
		else
			LOD_level = MAX_DEPTH - level - 1;
}

//copy constructor
QuadTree::QuadTree(const QuadTree& other){
	*this = other;
}

//copy assignment operator
QuadTree& QuadTree::operator=(const QuadTree& other){
	//data = other.data;
	level = other.level;
	box = other.box;
	for(int j = 0; j < 4; j++){
		nodes[j] = other.nodes[j];
	}
	has_data = other.has_data;
	has_children = other.has_children;
	n = other.n;
	LOD_level = other.LOD_level;
	//cam_pos = other.cam_pos;
	return *this;
}

QuadTree::~QuadTree(){
	clear();
}

//initialize the QuadTree 
void QuadTree::initCamPos(glm::vec3 camPos){
	//for(int i = 0; i < box.width; i++){
	//	for(int j = 0; j < box.height; j++){
	//		if((i == 0 && j == 0)	||	 (i == 0 && j == box.height) ||
	//			(i == box.width && j == 0)	|| (i == box.width && j == box.height)) {
	//				continue;
	//		}
	//			insert(i, j, 0);
	//	}
	//}
	//
	//cam_pos = camPos;
}

void QuadTree::initSelectionList(int listSize){
	if(selected.size() != listSize)
		selected.resize(listSize);
	//selected[0] = this;
	nodeCount = 0;
}

//Clears current node and all child nodes
void QuadTree::clear(){
	if(has_children){
		for(int i = 0; i < 4; i++){
			nodes[i]->clear();
		}
		delete[] nodes;
	}
}

//*******************************************************************
//TODO: Fix the order in which this method inserts into the quadtree.
//		Lower levels of quadtree (close to root) should only store a 
//		few points, enough for a rough outline of the mesh. Points 
//		in-between quadrant division lines are higher level

//		The quadtree should be able to hold multiple levels of detail
//		within the structure. The root node and its children represent 
//		the lowest detail (LOD=max) and the leaf nodes represent the 
//		highest detail (LOD=0)
//*******************************************************************
//Inserts a value in the quadtree
//INPUT:	DataPoint value, 
//			Bounding box representing the region
//			of the current node
void QuadTree::insert(int x, int z, float value){
	//if level < MAX_DEPTH, recurse to children
	if(level < MAX_DEPTH) {
		if(containsInCorners(x,z)){
			for(int i = 0; i < 4; i++){
				if(box[i].x == x && box[i].z == z){
					box[i].value = value;
					QuadTree::totalNodes++;
					break;
				}
			}
		}
		else if(contains(x,z)){
			split();
			for(int j = 0; j < 4; j++){
				nodes[j]->insert(x, z, value);
			}
		}
		else {
			//std::cout << "QuadTree: error! Value inserted outside of bounds" << std::endl;
			//event_log << "QuadTree: error! Value inserted outside of bounds" << std::endl;
			//system("pause");
			//exit(-1);
			return;
		}
		
	}
	else {
		for(int i = 0; i < 4; i++){
			if(box[i].x == x && box[i].z == z){
				box[i].value = value;
				QuadTree::totalNodes++;
				return;
			}
		}
	}
}

//If this node has no children, create 4 subnodes
// and fill each with a bounding box 1/4th the size
//	of the parent node
void QuadTree::split(){
	if(!has_children){

		//Define top-left corners of the quadrants
		DataPoint	topleftCorners[] = 
		{	
			DataPoint(box[NW].x, box[NW].z, -7777),					//NW corner
			DataPoint((box[NW].x + box.width/2),		//NE corner
						box[NW].z, -7777), 
			DataPoint(box[NW].x,								//SW corner
					   (box[NW].z + box.height/2), -7777), 
			DataPoint((box[NW].x + box.width/2),		//SE corner
					   (box[NW].z + box.height/2), -7777) 
		};

		//create 4 sub nodes
		for(int j = 0; j < 4; j++) {
			nodes[j] = new QuadTree( level+1, BoundingBox(box.width/2, box.height/2) );

			nodes[j]->box[NW] = topleftCorners[j];

			nodes[j]->box[NE].x = topleftCorners[j].x + box.width/2;
			nodes[j]->box[NE].z = topleftCorners[j].z;

			nodes[j]->box[SW].x = topleftCorners[j].x;
			nodes[j]->box[SW].z = topleftCorners[j].z + box.height/2;

			nodes[j]->box[SE].x = topleftCorners[j].x + box.width/2;
			nodes[j]->box[SE].z = topleftCorners[j].z + box.height/2;
			//nodes[j]->n = n/2;
			//nodes[j]->box.width = box.width/2;
			//nodes[j]->box.height = box.height/2;

			//copy corner values of current node to one of the corners 
			//of each child node
			switch(j){
			case NW:
				nodes[j]->box[NW].value = box[NW].value;
				break;
			case NE:
				nodes[j]->box[NE].value = box[NE].value;
				break;
			case SW:
				nodes[j]->box[SW].value = box[SW].value;
				break;
			case SE:
				nodes[j]->box[SE].value = box[SE].value;
				break;
			}
		}


		has_children = true;
	}
}

//*******************************
//TODO:	Selection Algorithm
//	Select nodes based on how far away they are
//	from the viewer. Dynamically render more nodes at
//	close distances to the viewer, and less nodes at
//	further distances 
//	ALSO, must keep a relatively constant number of
//	triangles on screen at any time
//*******************************

//Starting at LODmin (the highest LOD level), recursively select nodes 
//that are within range of the specified lodLevel
//INPUT:	float:	array of LOD range values to check against
//			int:	the LOD level to start at 
bool QuadTree::select(float *lodRanges, int lodLevel, glm::vec3 cam_pos, Frustum &cam_view){
	//if(lodLevel < 6)
	//	return false;

	//frustrum check
	if(!box.withinFrustum(cam_view)){
		//if(nodeCount == 0) selected[0] = this;
		return false;
	}

	//check if node is not entirely in range
	if(!box.withinRange(cam_pos, lodRanges[lodLevel], min_distance)){

		//if the node bounding box is not entirely in range of LOD level, return false
		return false;
	}

	//if this node is at LODmax and completely in range, add to selected list
	if(lodLevel == 0) {
		selected[nodeCount++] = this;
	}
	else {
		//if this is not LODmax, check if its eligible for more detailed LOD level
		//if bounding box is partially or not at all within range, add current node
		if(!box.withinRange(cam_pos, lodRanges[lodLevel-1], min_distance)){
			selected[nodeCount++] = this;
		}
		else {
			//bounding box is fully within range of lower LOD level
			//check each child node
			for(int k = 0; k < 4; k++){

				//
				if(!nodes[k]->select(lodRanges, lodLevel-1, cam_pos, cam_view)){
					selected[nodeCount++] = nodes[k];
				}
			}
		
		}
	
	}


	nodeCount = nodeCount%selected.size();

	return true;
}

//Returns TRUE if the current node or its children contain pt
//	FALSE otherwise
bool QuadTree::contains(int x, int z){
	return (x >= box[NW].x  &&  z >= box[NW].z  &&  
			x <= (box[NW].x + box.width)  &&  
			z <= (box[NW].z + box.height));
}

bool QuadTree::containsInCorners(int x, int z){
	if( (box[NW].x == x && box[NW].z == z) ||
		(box[NW].x + box.width == x && box[NW].z == z) ||
		(box[NW].x == x && box[NW].z + box.height == z) ||
		(box[NW].x + box.width == x && box[NW].z + box.height == z))
		return true;
	else
		return false;
}

//Renders the currently selected list of nodes
//OUTPUT:	Returns true if successfully rendered,
//			false otherwise
//bool QuadTree::render(){
//
//}

bool QuadTree::HasChildren(){
	return has_children;
}

bool QuadTree::HasData(){
	return has_data;
}

//	GET Methods

float QuadTree::getDataPointAt(int x, int z){
	//Recurse to the lowest level node
	if(level < MAX_DEPTH) {
		if(containsInCorners(x,z)) {
			for(int i = 0; i < 4; i++){
				if(box[i].x == x && box[i].z == z){
					return box[i].value;
				}
			}
		}
		else if(contains(x,z)) {
			for(int t = 0; t < 4; t++){
				if(nodes[t]->contains(x,z))
					return nodes[t]->getDataPointAt(x, z);
			}
		}
		else {
			std::cout << "QuadTree: Error! No value exists at point (" 
				<< x << ", " << z << ")" << std::endl;
			event_log << "QuadTree: Error! No value exists at point (" 
				<< x << ", " << z << ")" << std::endl;
			system("pause");
			exit(-1);
		}
	}
	//Reached the lowest level node (LOD max)
	else {
		for(int j = 0; j < 4; j++) {
			if(box[j].x == x && box[j].z == z)
				return box[j].value;
		}
		std::cout << "QuadTree: Error inputting value at (" 
			<< x << ", " << z << ")\n" 
			"Check that MAX_DEPTH corresponds to n"<< std::endl;
		event_log << "QuadTree: Error inputting value at (" 
			<< x << ", " << z << ")\n" 
			"Check that MAX_DEPTH corresponds to n"<< std::endl;
		system("pause");
		exit(-1);
	}
}

//BoundingBox QuadTree::getNodeAt(int LODLevel, int x, int z){
//	if()
//}

//returns the number of SELECTED nodes
int QuadTree::getNumSelected(){
	return nodeCount;
}

//returns the total number of nodes in this QuadTree
int QuadTree::getTotalNodes(){
	return totalNodes;
}

QuadTree **QuadTree::getSelectedNodes(){
	return selected.data();
}

BoundingBox &QuadTree::getBox(){
	return box;
}

//returns this node's LOD level
int QuadTree::getLOD(){
	return LOD_level;
}

//incomplete. Fix or remove.
float QuadTree::getMinDistance(){
	return min_distance;
}

//gets the exponent of a power-of-2 number
int logBase2(int num){
	int count = 0;
	while(num > 1){
		num = num/2;
		count++;
	}
	return count;
}
