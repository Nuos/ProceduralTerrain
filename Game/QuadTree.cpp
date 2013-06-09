#include "QuadTree.hpp"
#include "event_logger.h"
#include <iostream>

QuadTree::QuadTree(int level, BoundingBox box) 
	: data(), level(level), has_children(false), has_data(false), 
	box(box), n(box.width), state(NODE_UNSELECTED) {
	
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
	return *this;
}

QuadTree::~QuadTree(){
	for(int i = 0; i < 4; i++){
		delete nodes[i];
	}
}

//Clears current node and all child nodes
void QuadTree::clear(){
	//data.resize(0);
	
	if(has_children){
		for(int i = 0; i < 4; i++){
			nodes[i]->clear();
		}
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
void QuadTree::insert(int x, int y, float value){

	//if(x < 0 || y < 0
	//	|| x > box.width || y > box.height){
	//		
	//}

	//if level < MAX_DEPTH, recurse to children
	if(level < MAX_DEPTH-1) {
		split();
		if(nodes[NW]->contains(x, y))
			nodes[NW]->insert(x, y, value);

		else if(nodes[NE]->contains(x, y))
			nodes[NE]->insert(x, y, value);

		else if(nodes[SW]->contains(x, y))
			nodes[SW]->insert(x, y, value);

		else if(nodes[SE]->contains(x, y))
			nodes[SE]->insert(x, y, value);
		else {
			std::cout << "QuadTree: error! Value inserted outside of bounds" << std::endl;
			event_log << "QuadTree: error! Value inserted outside of bounds" << std::endl;
			system("pause");
			exit(-1);
		}

	}
	else {
		//level has reached MAX_DEPTH

		for(int i = 0; i < 4; i++){
			if(box[i].x == x && box[i].y == y){
				box[i].value = value;
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
			DataPoint(box[NW].x, box[NW].y, 0),					//NW corner
			DataPoint((box[NW].x + box.width/2)%box.width,		//NE corner
						box[NW].y, 0), 
			DataPoint(box[NW].x,								//SW corner
					   (box[NW].y + box.height/2)%box.height, 0), 
			DataPoint((box[NW].x + box.width/2)%box.width,		//SE corner
					   (box[NW].y + box.height/2)%box.height, 0) 
		};

		//create 4 sub nodes
		for(int j = 0; j < 4; j++) {
			nodes[j] = new QuadTree( level+1, BoundingBox(box.width/2, box.height/2) );

			nodes[j]->box[NW] = topleftCorners[j];

			nodes[j]->box[NE].x = topleftCorners[j].x + box.width/2;
			nodes[j]->box[NE].y = topleftCorners[j].y;

			nodes[j]->box[SW].x = topleftCorners[j].x;
			nodes[j]->box[SW].y = topleftCorners[j].y + box.height/2;

			nodes[j]->box[SE].x = topleftCorners[j].x + box.width/2;
			nodes[j]->box[SE].y = topleftCorners[j].y + box.height/2;
			nodes[j]->n = n/2;
		}


		has_children = true;
	}
}

//Select nodes that satisfy a certain condition
//INPUT:	float distance
QuadTree QuadTree::select(float distance){

	//every node that is less than or equal to this distance is selected
	if(level < MAX_DEPTH){

	}
	else if(has_data) {
		state = NODE_SELECTED;
	}
	else {
		std::cout << "Error! Node has no data" << std::endl;
	}
	return *this;
}

//Returns TRUE if the current node or its children contain pt
//	FALSE otherwise
bool QuadTree::contains(int x, int y){
	return (x >= box[NW].x || 
			y >= box[NW].y || 
			x < box.width || 
			y < box.height);
}

bool QuadTree::HasChildren(){
	return has_children;
}

bool QuadTree::HasData(){
	return has_data;
}

//	GET Methods

float QuadTree::getDataPointAt(int x, int y){
	return 0;
}

int QuadTree::getNumSelected(){
	return num_selected;
}