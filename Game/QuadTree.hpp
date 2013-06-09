#ifndef __QUAD_TREE_H_
#define __QUAD_TREE_H_
#include <vector>
#include <glm/glm.hpp>

enum Quadrant {
	NW,
	NE,
	SW,
	SE,
	NumQuadrants
};

enum NodeState {
	NODE_UNSELECTED,
	NODE_SELECTED
};

struct DataPoint {
public:
	DataPoint::DataPoint() :
		x(0), y(0), value(0){}
	DataPoint::DataPoint(int x, int y, float val) : 
		x(x), y(y), value(val){}

	DataPoint::DataPoint(const DataPoint& other){
		*this = other;
	}

	DataPoint& DataPoint::operator=(const DataPoint& rhs) {
		x = rhs.x;
		y = rhs.y;
		value = rhs.value;
		return *this;
	}

	bool DataPoint::isEmpty(){
		if(x == 0 && y == 0 && value == 0)
			return true;
		else 
			return false;
	}

	int x, y;
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

	//Split the current bounding box into 4 smaller ones
	//INPUT:	Enum specifying which quadrant to create
	//OUTPUT:	BoundingBox of one quadrant
	//BoundingBox& BoundingBox::splitToQuads(Quadrant qq){
	//	BoundingBox quadrant;
	//	quadrant.width = this->width/2;
	//	quadrant.height = this->height/2;
	//	int x, y;

	//	switch(qq) {
	//	case NW:
	//		x = corners[qq].x;
	//		y = corners[qq].y;
	//		break;
	//	case NE:
	//		x = corners[qq].x + this->width;
	//		y = corners[qq].y;
	//		break;
	//	case SW:
	//		x = corners[qq].x;
	//		y = corners[qq].y + this->height;
	//		break;
	//	case SE:
	//		x = corners[qq].x + this->width;
	//		y = corners[qq].y + this->height;
	//		break;
	//	default:
	//		break;
	//	}
	//	quadrant.corners[qq].x = x;
	//	quadrant.corners[qq].y = y;

	//	return quadrant;
	//}

	DataPoint corners[NumQuadrants];
	int width, height;
};



/*
	QUADTREE class

	Description: 
		A Quad Tree holds MAX_POINTS floating point values per node.
		If any node contains more than MAX_POINTS values, that node is 
		split into 4 child nodes.

		For an adaptive quadtree, MAX_POINTS is set to 0
*/

class QuadTree {

public:
	QuadTree::QuadTree(int level, BoundingBox boundBox);

	QuadTree::~QuadTree();

	//copy constructor
	QuadTree::QuadTree(const QuadTree& rhs);

	//copy assignment operator
	QuadTree& QuadTree::operator=(const QuadTree &rhs);


	//*****************************
	//	Node management functions
	//*****************************

	void QuadTree::clear();

	//Inserts a value into the current node
	void QuadTree::insert(int x, int y, float value);
	//void QuadTree::updateNode(int level, std::vector<DataPoint> val, BoundingBox box);

	//Split current node into 4 child nodes
	//	In other words, split the current box into 
	//	4 quadrants
	void QuadTree::split();

	//Selects a sub-tree with nodes within the specified distance
	//INPUT:	float distance from node to the viewer's eye
	QuadTree QuadTree::select(float distance);

	//Returns TRUE if the current node or its children contain pt
	//FALSE otherwise
	//INPUT:	x, y coordinates to check
	bool QuadTree::contains(int x, int y);


	//***************
	//	Get methods
	//***************

	float QuadTree::getDataPointAt(int x, int y);
	int QuadTree::getCurrentDepth();
	int QuadTree::getNumSelected();


	//*****************
	//	status methods
	//*****************

	bool QuadTree::HasChildren();
	bool QuadTree::HasData();


private:

	//each node points to up to 4 child nodes
	QuadTree *nodes[4];

	//need a bounding box for each node
	//glm::vec4 boundingBox;

	DataPoint data;

	bool has_children, has_data;
	
	BoundingBox box;
	int level;		//the current depth level
	int n;			// n value, where the grid to be rendered
					// is (n+1) x (n+1)
	NodeState state;
	int num_selected;	//the number of elements contained within this node and all child nodes
	static const int MAX_DEPTH = 8;
	static const int MAX_POINTS = 1;	//for a heightmap, one value
										//per LEAF node
};

#endif