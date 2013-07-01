#ifndef __DISPLACE_TERRAIN_H_
#define __DISPLACE_TERRAIN_H_
#include <SFML/System/Vector2.hpp>
#include <vector>
#include "Mesh.hpp"
#include "mtrand.h"
#include "QuadTree.hpp"

using namespace std;
using namespace sf;

enum PointType {
	PT_VERTEX,
	PT_NORMAL,
	PT_TEXTURE,
	PT_UNIFORM,
	NUM_PT
};

//glm::vec3 cross(float ax,float ay, float az, float bx, float by, float bz);
//glm::vec3 cross(glm::vec3 a, glm::vec3 b);

class DisplaceTerrain : public Mesh{
public:
	typedef Mesh superduper;

	enum cardinal {NORTH, SOUTH, EAST, WEST, 
				   NORTHEAST, NORTHWEST, SOUTHEAST, SOUTHWEST, NONE};
	enum diamond_square {DIAMOND_STEP, SQUARE_STEP};
	DisplaceTerrain::DisplaceTerrain(int n, float height, float reduction, float tile_size, Camera *cam);
	DisplaceTerrain::DisplaceTerrain(int n, glm::vec3 gradient, float tile_size);
	DisplaceTerrain::~DisplaceTerrain();

	//copy constructor
	DisplaceTerrain::DisplaceTerrain(const DisplaceTerrain& rhs);

	//copy assignment operator
	DisplaceTerrain DisplaceTerrain::operator=(const DisplaceTerrain& rhs);

	void DisplaceTerrain::initLOD();

	//set camera: required for quadtree
	void DisplaceTerrain::setCamera(Camera *cam);

	//void DisplaceTerrain::loadTexture(const char* filename, GLenum type);

	void DisplaceTerrain::setSeed(int seed);
	void DisplaceTerrain::construct(bool drawOutline);
	void DisplaceTerrain::construct(int n, float height, float reduction, float tile_size, bool drawOutline);
	void DisplaceTerrain::constructPerlin2D();
	
	void DisplaceTerrain::updateVertexArray(bool drawOutline);

	void DisplaceTerrain::updateCamProperties(Frustum &cam_view);
	
	friend float avg(float num, ...);

	void DisplaceTerrain::print();

	//void DisplaceTerrain::IncreaseDetail(){
	//	if((n/LOD_factor) > 1)
	//		LOD_factor++;
	//	construct();
	//}

	//void DisplaceTerrain::DecreaseDetail(){
	//	if(LOD_factor > 1)
	//		LOD_factor--;
	//	construct();
	//}

	//	GET functions
	int DisplaceTerrain::getNumTriangles();

	float DisplaceTerrain::getCenterDistance();
	
private:

	void DisplaceTerrain::buildGradientMap();

	float DisplaceTerrain::getPseudoRand(float max, float min);

	glm::vec3 DisplaceTerrain::getRandGradient();

	//construct terrain using diamond square algorithm
	//INPUT:	coordinates of top left corner 
	//			the width of the current square
	//			the Cardinal direction of which points to omit
	void DisplaceTerrain::construct(Vector2i p00, int width, float tHeight);

	//generates a terrain mesh from map data
	void DisplaceTerrain::updateMesh();

	//Renders the currently selected list of nodes
	void DisplaceTerrain::updateQuadMesh(bool drawOutline);


	//adds x, y, z(optional) values to vertex arrays
	//INPUT:	glm::vec3:	the point to insert
	//			PointType:	the type of the point - vertex, normal, or texture
	//			int:	index of the vertex array
	//			int:	offset of index
	void DisplaceTerrain::createPoint(glm::vec3 pt, PointType type, int index, int offset);

	//creates one square tile, composed of two triangles
	//and saves it in vertex, normal, and texture arrays
	//INPUT:	glm::vec3: coordinates of each corner,
	//			int: n - this is just used to differentiate between rendering 
	//					from nodelist and rendering from heightmap
	//			int: (2D) takes x and y coordinate to generate tile from 2D array
	//			int: (1D) if yDim not specified, 1D array assumed
	void DisplaceTerrain::createTile(glm::vec3 &topleft, glm::vec3 &topright, 
									glm::vec3 &botleft, glm::vec3 &botright,
									std::vector<glm::vec3> &normals, bool drawOutline,
									int LOD_level, int yDim, int xDim = 0, int n = 1);

	//creates one square tile, rendered as an outline
	//and saves it in vertex, normal, and texture arrays
	//INPUT:	glm::vec3: coordinates of each corner,
	//			int: n - this is just used to differentiate between rendering 
	//					from nodelist and rendering from heightmap
	//			int: (2D) takes x and y coordinate to generate tile from 2D array
	//			int: (1D) if yDim not specified, 1D array assumed
	//void DisplaceTerrain::createLineTile(glm::vec3 &topleft, glm::vec3 &topright, 
	//									 glm::vec3 &botleft, glm::vec3 &botright, 
	//									 std::vector<glm::vec3> &normals, int xDim, int yDim = 0,  int n = 1);

	void DisplaceTerrain::calcNormals(std::vector<glm::vec3> &normals, int yDim, int xDim = 0, int n = 1);

	//Camera properties
	//glm::vec3 camPos;
	//glm::mat4 viewMat;
	Frustum camView;

	MTRand_closed rand_height;
	int n, seed, LOD_Levels;
	bool seedSet;
	float height, reduction, tile_size;
	float *lod_ranges;
	float min_distance;
	//float **map;
	vector< vector<float> > map;
	vector< vector<glm::vec2> > gradient_map;
	QuadTree *root;
};

#endif