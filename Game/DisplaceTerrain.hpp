#ifndef __DISPLACE_TERRAIN_H_
#define __DISPLACE_TERRAIN_H_
#include <SFML/System/Vector2.hpp>
#include <vector>
#include "Mesh.hpp"
#include "mtrand.h"
#include "QuadTree.hpp"

using namespace std;
using namespace sf;

glm::vec3 cross(float ax,float ay, float az, float bx, float by, float bz);
glm::vec3 cross(glm::vec3 a, glm::vec3 b);

class DisplaceTerrain : public Mesh{
public:
	typedef Mesh superduper;

	enum cardinal {NORTH, SOUTH, EAST, WEST, 
				   NORTHEAST, NORTHWEST, SOUTHEAST, SOUTHWEST, NONE};
	enum diamond_square {DIAMOND_STEP, SQUARE_STEP};
	DisplaceTerrain::DisplaceTerrain(int n, float height, float reduction, float tile_size);
	DisplaceTerrain::DisplaceTerrain(int n, glm::vec3 gradient, float tile_size);
	DisplaceTerrain::~DisplaceTerrain();

	//copy constructor
	DisplaceTerrain::DisplaceTerrain(const DisplaceTerrain& rhs){
		//superduper(rhs);
		*this = rhs;
	}

	//copy assignment operator
	DisplaceTerrain DisplaceTerrain::operator=(const DisplaceTerrain& rhs){
		superduper::operator=(rhs);
		n = rhs.n;
		height = rhs.height;
		reduction = rhs.reduction;
		tile_size = rhs.tile_size;
		map = rhs.map;

		return *this;
	}

	//void DisplaceTerrain::loadTexture(const char* filename, GLenum type);

	void DisplaceTerrain::setSeed(int seed);
	void DisplaceTerrain::construct();
	void DisplaceTerrain::construct(int n, float height, float reduction, float tile_size);
	void DisplaceTerrain::constructPerlin2D();

	friend float avg(float num, ...);

	void DisplaceTerrain::print();

	
private:

	void DisplaceTerrain::buildGradientMap();

	float DisplaceTerrain::getPseudoRand(float max, float min);

	glm::vec3 DisplaceTerrain::getRandGradient();

	//construct terrain using diamond square algorithm
	//INPUT:	coordinates of top left corner 
	//			the width of the current square
	//			the Cardinal direction of which points to omit
	void DisplaceTerrain::construct(Vector2i p00, int width, float theight);

	//generates a terrain mesh from map data
	void DisplaceTerrain::updateMesh();

	void DisplaceTerrain::updateQuadMesh();

	MTRand_closed rand_height;
	int n, seed;
	bool seedSet;
	float height, reduction, tile_size;
	//float **map;
	vector< vector<float> > map;
	vector< vector<glm::vec2> > gradient_map;
	QuadTree *root;
};

#endif