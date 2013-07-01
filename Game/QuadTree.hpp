#ifndef __QUAD_TREE_H_
#define __QUAD_TREE_H_
#include <vector>
#include "Camera.hpp"

enum Quadrant {
	NW,
	NE,
	SW,
	SE,
	NumQuadrants,
};

enum QuadColors {
	QUAD_WHITE,
	QUAD_BLUE,
	QUAD_RED,
	QUAD_GREEN,
	QUAD_PURP,
	NUM_QUAD_COLORS
};

//gets the exponent of a power-of-2 number
extern int logBase2(int num);

struct DataPoint {
public:
	DataPoint::DataPoint() :
		x(0), z(0), value(0) {}
	DataPoint::DataPoint(int x, int z, float val) : 
		x(x), z(z), value(val) {}

	DataPoint::DataPoint(const DataPoint& other){
		*this = other;
	}

	DataPoint& DataPoint::operator=(const DataPoint& rhs) {
		x = rhs.x;
		z = rhs.z;
		value = rhs.value;
		return *this;
	}

	glm::vec3 DataPoint::getVec3(){
		return glm::vec3(x, value, z);
	}

	bool DataPoint::isEmpty(){
		return (value == -7777);
	}

	int x, z;
	float value;
};

struct BoundingBox {
public:
	//default constructor
	BoundingBox::BoundingBox()
		: width(0), height(0) {}

	BoundingBox::BoundingBox(int width, int height)
		: width(width), height(height) {}

	BoundingBox::BoundingBox(DataPoint values[], int width, int height)
		: width(width), height(height) {
			corners[NW] = values[NW];
			corners[NE] = values[NE];
			corners[SW] = values[SW];
			corners[SE] = values[SE];
	}

	//INPUT:	int x, 
	//	int y,
	//	int value for width and height
	BoundingBox::BoundingBox(DataPoint values[], int width)
		: width(width), height(width) {
			corners[NW] = values[NW];
			corners[NE] = values[NE];
			corners[SW] = values[SW];
			corners[SE] = values[SE];
	}

	DataPoint& BoundingBox::operator[](const int index){
		return corners[index];
	}

	//if the distance between camera position and furthest point on bounding box
	//is within range of LOD_range, return true
	//	otherwise return false
	bool BoundingBox::withinRange(glm::vec3 cam_pos, float LOD_range, float &min_distance){
		for(int i = 0; i < 4; i++){
			float distBoxToCamPos = glm::length(corners[i].getVec3() - cam_pos);
			if(min_distance > distBoxToCamPos)
					min_distance = distBoxToCamPos;
			if(distBoxToCamPos > LOD_range) {
				return false;
			}
		}
		return true;
	}

	//if at least one corner of node is within the viewing frustum, return true
	bool BoundingBox::withinFrustum(Frustum &cam_view){
		if(!cam_view.init)
			return false;
		
		//for each of the 6 planes, check whether bounding box is in front or behind plane
		for(int j = 0; j < 6; j++){
			int pointsBehindPlane = 0;
			//Iterate through each corner of node
			for(int i = 0; i < 4; i++){
				if(cam_view.planes[j].signedDistance(corners[i].getVec3()) < 0){
					pointsBehindPlane++;
				}
			}
			//if all box points are behind one plane, it is outside the frustum
			if(pointsBehindPlane == 4)
				return false;
		}
		return true;
	}

	//returns true if all corners have values
	bool BoundingBox::isFull(){
		for(int i = 0; i < 4; i++){
			if( corners[i].isEmpty() )
				return false;
		}
		return true;
	}

	DataPoint corners[NumQuadrants];
	int width, height;
};


/*
	QUADTREE class

*/

class QuadTree {

public:
	QuadTree::QuadTree(int level, BoundingBox boundBox);

	QuadTree::~QuadTree();

	//copy constructor
	QuadTree::QuadTree(const QuadTree& rhs);

	//copy assignment operator
	QuadTree& QuadTree::operator=(const QuadTree &rhs);

	//initialize QuadTree
	void QuadTree::initCamPos(glm::vec3 camPos);

	void QuadTree::initSelectionList(int listSize);


	//*****************************
	//	Node management functions
	//*****************************

	void QuadTree::clear();

	//Inserts a value into the current node
	void QuadTree::insert(int x, int z, float value);
	
	//void QuadTree::updateNode(int level, std::vector<DataPoint> val, BoundingBox box);

	//void QuadTree::insertValue(float value) 

	//Split current node into 4 child nodes
	//	In other words, split the current box into 
	//	4 quadrants
	void QuadTree::split();

	//Selects a sub-tree with nodes within the specified distance
	//INPUT:	float distance from node to the viewer's eye
	//			LOD level to render up to
	bool QuadTree::select(float *lodRanges, int lodLevel, glm::vec3 cam_pos, Frustum &cam_view);

	//Returns TRUE if the current node or its children contain pt
	//FALSE otherwise
	//INPUT:	x, y coordinates to check
	bool QuadTree::contains(int x, int z);

	//Returns TRUE if point (x,z) is one of this node's corners
	//INPUT:	x, y coordinates to check
	bool QuadTree::containsInCorners(int x, int z);

	//Renders the currently selected list of nodes
	//OUTPUT:	Returns true if successfully rendered,
	//			false otherwise
	bool QuadTree::render();


	//***************
	//	Get methods
	//***************

	float QuadTree::getDataPointAt(int x, int z);
	BoundingBox QuadTree::getNodeAt(int LODLevel, int x, int z);
	int QuadTree::getCurrentDepth();
	int QuadTree::getNumSelected();
	int QuadTree::getTotalNodes();
	static QuadTree **QuadTree::getSelectedNodes();
	BoundingBox &QuadTree::getBox();
	int QuadTree::getLOD();
	float QuadTree::getMinDistance();

	//*****************
	//	status methods
	//*****************

	bool QuadTree::HasChildren();
	bool QuadTree::HasData();


private:

	//each node points to up to 4 child nodes
	QuadTree *nodes[4];

	//Camera position
	//glm::vec3 cam_pos;

	//need a bounding box for each node
	//glm::vec4 boundingBox;

	//DataPoint data;
	static std::vector<QuadTree*> selected;

	bool has_children, has_data;
	
	BoundingBox box;
	int level;		//the current depth level
	int n;			// n value, where the grid to be rendered
					// is (n+1) x (n+1)
	int LOD_level;
	//NodeState state;
	//int num_selected;	//the number of elements contained within this node and all child nodes
	static int totalNodes;
	static int nodeCount;
	static int MAX_DEPTH;
	static const int MAX_POINTS = 1;	//for a heightmap, one value
										//per LEAF node

	//Debugging variables
	static float min_distance;
};

#endif