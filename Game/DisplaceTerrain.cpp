#include "DisplaceTerrain.hpp"
#include "event_logger.h"
#include <ctime>
#include <cstdlib>
#include <cmath>
#include <cstdarg>
#include <iostream>
#include <iomanip>
#include <glm/gtc/matrix_transform.hpp>


float length(glm::vec3 vec){
	float num = vec.x * vec.x + vec.y * vec.y + vec.z * vec.z;
	return sqrtf(num);
}

glm::vec3 getAvgNormal(vector<glm::vec3> &normals, int i, int j, int n){
	glm::vec3 ans;
	ans = 
		normals[(6*n*(i)+(6*n*n))%(6*n*n) +	  (6*(j)+6*n)%(6*n)] +			//square (i,j)
		normals[(6*n*(i)+(6*n*n))%(6*n*n) +   (6*(j-1)+6*n)%(6*n)+5] +		//square (i,j-1) + 5
		normals[(6*n*(i)+(6*n*n))%(6*n*n) +   (6*(j-1)+6*n)%(6*n)+2] +		//square (i,j-1) + 2
		normals[(6*n*(i-1)+(6*n*n))%(6*n*n) + (6*(j-1)+6*n)%(6*n)+4] +		//sqaure (i-1,j-1) + 4
		normals[(6*n*(i-1)+(6*n*n))%(6*n*n) + (6*(j)+6*n)%(6*n)+1] + 		//square (i-1,j) + 1
		normals[(6*n*(i-1)+(6*n*n))%(6*n*n) + (6*(j)+6*n)%(6*n)+3];		//square (i-1,j) + 3
	ans /= 6;
	normalize(ans);
	return ans;
}

//glm::vec3 getQuadNormal(vector<glm::vec3> &normals, int i, int j) {
//	glm::vec3 ans;
//	ans = 
//		normals[(6*(i)+(6))%(6) +	  (6*(j)+6)%(6)] +			//square (i,j)
//		normals[(6*(i)+(6))%(6) +   (6*(j-1)+6)%(6)+5] +		//square (i,j-1) + 5
//		normals[(6*(i)+(6))%(6) +   (6*(j-1)+6)%(6)+2] +		//square (i,j-1) + 2
//		normals[(6*(i-1)+(6))%(6) + (6*(j-1)+6)%(6)+4] +		//sqaure (i-1,j-1) + 4
//		normals[(6*(i-1)+(6))%(6) + (6*(j)+6)%(6)+1] + 		//square (i-1,j) + 1
//		normals[(6*(i-1)+(6))%(6) + (6*(j)+6)%(6)+3];		//square (i-1,j) + 3
//	ans /= 6;
//	normalize(ans);
//	return ans;
//}


DisplaceTerrain::DisplaceTerrain(int n, float height, float reduction, float tile_size, Camera *cam) 
	: n(n), height(height), reduction(reduction), tile_size(tile_size), seed(-999),
	seedSet(false), root(NULL), LOD_Levels(logBase2(n)+1),
	camView(cam->getFrustum()), min_distance(9999) {

	map.resize(n+1);
	for(int j = 0; j < n+1; j++){
		map[j].resize(n+1);
	}
	//seed = 2314;
	//seedSet = true;

	//initialize corner values
	map[0][0] = height;
	map[0][n] = height;
	map[n][0] = height;
	map[n][n] = height;

	initLOD();
}

//copy constructor
DisplaceTerrain::DisplaceTerrain(const DisplaceTerrain& rhs){
	*this = rhs;
}

//copy assignment operator
DisplaceTerrain DisplaceTerrain::operator=(const DisplaceTerrain& rhs){
	superduper::operator=(rhs);
	n = rhs.n;
	seed = rhs.seed;
	seedSet = rhs.seedSet;
	LOD_Levels = rhs.LOD_Levels;
	height = rhs.height;
	reduction = rhs.reduction;
	tile_size = rhs.tile_size;
	min_distance = rhs.min_distance;

	for(int i = 0; i < map.size(); i++){
		for(int j = 0; j < map[i].size(); j++){ 
			map[i][j] = rhs.map[i][j];
		}
	}
	for(int i = 0; i < gradient_map.size(); i++){
		for(int j = 0; j < gradient_map[i].size(); j++){ 
			gradient_map[i][j] = rhs.gradient_map[i][j];
		}
	}

	for(int k = 0; k < LOD_Levels; k++){
		lod_ranges[k] = rhs.lod_ranges[k];
	}

	for(int p = 0; p < root->getTotalNodes(); p++){
		root[p] = rhs.root[p];
	}

	return *this;
}

void DisplaceTerrain::initLOD(){
	lod_ranges = new float[LOD_Levels];
	lod_ranges[0] = n * (8.0f * tile_size) / 128;
	for(int i = 1; i < LOD_Levels; i++){
		lod_ranges[i] = lod_ranges[i-1] * 2;
	}
}

//void DisplaceTerrain::setCamera(Camera *cam){
//	root->initCam(cam);
//}

DisplaceTerrain::~DisplaceTerrain(){
	//for(int i = 0; i < n+1; i++){
	//	delete[] map[i];
	//}
	//delete[] map;
	delete[] lod_ranges;
	delete root;
}


void DisplaceTerrain::setSeed(int seed){
	if(seed != this->seed){
		this->seed = seed;
		rand_height.seed(seed);
	}
	seedSet = true;
}

void DisplaceTerrain::construct(int n, float height, float reduction, float tile_size, bool drawOutline){
	if(this->n < n){
		//reinit 2d array

		//resize old array
		map.resize(n+1);
		for(int j = 0; j < n+1; j++){
			map[j].resize(n+1);
		}
	} 
	this->n = n;
	this->height = height;
	this->reduction = reduction;
	this->tile_size = tile_size;
	construct(drawOutline);
}

//generates a terrain mesh from map data
void DisplaceTerrain::updateMesh(){

	//create a flat plane of size (n+1)x(n+1)
	//first create an empty vertex array
	g_vert_count = 2*n*n*3;
	g_vp.resize(g_vert_count*3);
	g_vn.resize(g_vert_count*3);
	vector<glm::vec3> normals(g_vert_count);
	if(textureLoaded)
		g_vt.resize(g_vert_count*2);

	int mapWidth = n/(root->getLOD()+1);

	// n x n grid mesh has n*n squares, and (n+1)*(n+1) vertices
	//However, each square is composed of 2 triangles, and each
	//	triangle is composed of 3 vertices. So, g_vp will have 3x the 
	//	total number of triangles, which is always: n*n*2.
	//For each square, draw 2 triangles
	for(int i = 0; i < n; i++) {
		for(int j = 0; j < n; j++) {
			glm::vec3 topleft(	j*tile_size, 
								root->getDataPointAt(j, i), 
								i*tile_size);
			glm::vec3 botleft(	j*tile_size, 
								root->getDataPointAt(j, i+1), 
								(i+1)*tile_size);

			glm::vec3 topright(	(j+1)*tile_size,
								root->getDataPointAt(j+1, i),
								i*tile_size);

			glm::vec3 botright(	(j+1)*tile_size,
								root->getDataPointAt(j+1, i+1),
								(i+1)*tile_size);

			createTile(topleft, topright, botleft, botright, normals, false, root->getLOD(), i, j, n);
			
		}
	}

	//calculate the normals
	for(int i = 0; i < n; i++) {
		for(int j = 0; j < n; j++){
			calcNormals(normals, j, i, n);
		}
	}
	//baseModelMatrix = glm::mat4(1.0);
	//baseModelMatrix = glm::translate( baseModelMatrix, glm::vec3(-(n*tile_size)/2, -3.0f, -(n*tile_size)/2) );
	//eng::quat rot90;
	//rot90.buildFromAxis(glm::vec3(1.0f,0.0f,0.0f), 90.0f);
	//baseModelMatrix = rot90.getMatrix() * baseModelMatrix;
	//modelMatrix = apply_model_mat * baseModelMatrix;
	//diffuse_reflectance = glm::vec3(0.7, 0.7, 0.3);
	meshLoaded = true;
}

void DisplaceTerrain::updateQuadMesh(bool drawOutline){
	//float lod_ranges[] = {10, 20, 40, 80, 160, 320, 640};
	root->initSelectionList(n*n);
	//root->initCamPos(camPos);
	root->select(lod_ranges, LOD_Levels-1, *camView.origin, camView);
	QuadTree **selection = root->getSelectedNodes();
	//if(root->getNumSelected() == 0){
	//	std::cout << "updateQuadMesh: ERROR! No nodes selected" << std::endl;
	//	event_log << "updateQuadMesh: ERROR! No nodes selected" << std::endl;
	//	system("pause");
	//	exit(-1);
	//}
	int renderSize = root->getNumSelected();
	if(drawOutline)
		g_vert_count = renderSize*16;
	else
		g_vert_count = renderSize*6;
	g_vp.resize(g_vert_count*3);
	g_vn.resize(g_vert_count*3);
	if(textureLoaded)
		g_vt.resize(g_vert_count*2);
	std::vector<glm::vec3> normals(g_vert_count);
	for(int j = 0; j < renderSize; j++){
		glm::vec3 topleft(selection[j]->getBox()[NW].getVec3());

		glm::vec3 topright(selection[j]->getBox()[NE].getVec3());

		glm::vec3 botleft(selection[j]->getBox()[SW].getVec3());

		glm::vec3 botright(selection[j]->getBox()[SE].getVec3());

		createTile(topleft, topright, botleft, botright, normals, drawOutline, selection[j]->getLOD(), j); 

	}

	//calculate the normals
	for(int i = 0; i < renderSize; i++) {
		calcNormals(normals, 0, i, 1);
	}
	//baseModelMatrix = glm::mat4(1.0);
	//baseModelMatrix[3] = glm::vec4(-(n*tile_size)/2, -3.0, -(n*tile_size)/2, 1.0);
	//modelMatrix = apply_model_mat * baseModelMatrix;
	////diffuse_reflectance = glm::vec3(0.7, 0.7, 0.3);
	meshLoaded = true;
}


void DisplaceTerrain::createTile(glm::vec3 &topleft, glm::vec3 &topright, glm::vec3 &botleft, 
	glm::vec3 &botright, std::vector<glm::vec3> &normals, bool drawOutline, int LOD_level, int yDim, int xDim,  int n){
	
	int interval, offset = 0;
	float oldHeights[4];
	glm::vec3 *corners = new glm::vec3[4];
	

	if(!drawOutline) interval = 18;
	else { 
		interval = 48; 		
		oldHeights[0] = topleft.y;
		oldHeights[1] = botleft.y;
		oldHeights[2] = botright.y;
		oldHeights[3] = topright.y;
		topleft.y = height*(LOD_level+1);
		botleft.y = height*(LOD_level+1);
		botright.y = height*(LOD_level+1);
		topright.y = height*(LOD_level+1);
		corners[0] = topleft;
		corners[1] = botleft;
		corners[2] = botright;
		corners[3] = topright;
	}


	//calculate face normals

	// triangle 1 normals: (topright - botleft) X (topleft - botleft)
	glm::vec3 normal1 = crossProduct( (topright - botleft) , (topleft - botleft) );
	normalize(normal1);

	// triangle 2 normals: (botright - botleft) X (topright - botleft)
	glm::vec3 normal2 = crossProduct( (botright - botleft) , (topright - botleft) );
	normalize(normal2);


	
	//***************
	//	TRIANGLE 1
	//***************


	//top left corner 
	createPoint(topleft, PT_VERTEX, (interval*n*yDim)+(interval*xDim), offset);
	normals[(6*n*yDim)+(6*xDim) + offset] = normal1;	
	if (textureLoaded) {
		createPoint( glm::vec3(xDim*(1.0f/n), yDim*(1.0f/n), 0.0f), 
					PT_TEXTURE, (12*n*yDim)+(12*xDim), offset );
	}
	offset++;

	
	//bottom left corner
	createPoint(botleft, PT_VERTEX, (interval*n*yDim)+(interval*xDim), offset);
	normals[(6*n*yDim)+(6*xDim)+offset] = normal1;
	if(textureLoaded) {
		createPoint( glm::vec3(xDim*(1.0f/n), (yDim+1)*(1.0f/n), 0.0f), 
					PT_TEXTURE, (12*n*yDim)+(12*xDim), offset );
	}
	offset++;


	if(!drawOutline) {
		//top right corner
		createPoint(topright, PT_VERTEX, (interval*n*yDim)+(interval*xDim), offset);
		normals[(6*n*yDim)+(6*xDim)+offset] = normal1;
		if(textureLoaded) {
			createPoint( glm::vec3((xDim+1)*(1.0f/n), yDim*(1.0f/n), 0.0f), 
					PT_TEXTURE, (12*n*yDim)+(12*xDim), offset );
		}
		offset++;
	}



	//***************
	//	TRIANGLE 2
	//***************


	//bottom left corner
	createPoint(botleft, PT_VERTEX, (interval*n*yDim)+(interval*xDim), offset);
	normals[(6*n*yDim)+(6*xDim)+offset] = normal2;
	if(textureLoaded) {
		createPoint( glm::vec3(xDim*(1.0f/n), (yDim+1)*(1.0f/n), 0.0f), 
			PT_TEXTURE, (12*n*yDim)+(12*xDim), offset );
	}
	offset++;
	

	//bottom right corner
	createPoint(botright, PT_VERTEX, (interval*n*yDim)+(interval*xDim), offset);
	normals[(6*n*yDim)+(6*xDim)+offset] = normal2;
	if(textureLoaded) {
		createPoint( glm::vec3((xDim+1)*(1.0f/n), (yDim+1)*(1.0f/n), 0.0f), 
					PT_TEXTURE, (12*n*yDim)+(12*xDim), offset );
	}
	offset++;


	if(drawOutline){
		//bottom right corner
		createPoint(botright, PT_VERTEX, (interval*n*yDim)+(interval*xDim), offset++);
	}

	//top right corner
	createPoint(topright, PT_VERTEX, (interval*n*yDim)+(interval*xDim), offset);
	normals[(6*n*yDim)+(6*xDim)+offset] = normal2;
	if(textureLoaded) {
		createPoint( glm::vec3((xDim+1)*(1.0f/n), yDim*(1.0f/n), 0.0f), 
					PT_TEXTURE, (12*n*yDim)+(12*xDim), offset );
	}
	offset++;

	if(drawOutline) {
		// top right and top left for the last line segment

		//top right corner
		createPoint(topright, PT_VERTEX, (interval*n*yDim)+(interval*xDim), offset++);

		//top left corner 
		createPoint(topleft, PT_VERTEX, (interval*n*yDim)+(interval*xDim), offset++);
	}

	//DEBUGGING:	draw line segments from debugging mesh to terrain
	if(drawOutline){
		for(int k = 0; k < 4; k++){
			createPoint(corners[k], PT_VERTEX, (interval*n*yDim)+(interval*xDim), offset++);
			corners[k].y = oldHeights[k];
			createPoint(corners[k], PT_VERTEX, (interval*n*yDim)+(interval*xDim), offset++);
		}
	}

	delete corners;
}

void DisplaceTerrain::calcNormals(std::vector<glm::vec3> &normals, int xDim, int yDim, int n){

	glm::vec3 avgNormal;
	int offset = 0;

	//***************
	//	Triangle 1
	//***************

	for(int j = 0; j < 6; j++){
		//top left
		avgNormal = normals[6*n*yDim + j];
		createPoint(avgNormal, PT_NORMAL, (18*n*yDim)+(18*xDim), j);
	}


	/*
	//bottom left
	avgNormal = getAvgNormal(normals,(yDim+1),xDim,n);
	createPoint(avgNormal, PT_NORMAL, (18*n*yDim)+(18*xDim), offset++);

	//top right
	avgNormal = getAvgNormal(normals,yDim,(xDim+1),n);
	createPoint(avgNormal, PT_NORMAL, (18*n*yDim)+(18*xDim), offset++);


	//***************
	//	Triangle 2
	//***************

	//bottom left
	avgNormal = getAvgNormal(normals,(yDim+1),xDim,n);
	createPoint(avgNormal, PT_NORMAL, (18*n*yDim)+(18*xDim), offset++);

	//bottom right
	avgNormal = getAvgNormal(normals,(yDim+1),(xDim+1),n);
	createPoint(avgNormal, PT_NORMAL, (18*n*yDim)+(18*xDim), offset++);

	//top right
	avgNormal = getAvgNormal(normals,yDim,(xDim+1),n);
	createPoint(avgNormal, PT_NORMAL, (18*n*yDim)+(18*xDim), offset++);
	*/
}

void DisplaceTerrain::createPoint(glm::vec3 pt, PointType type, int index, int offset){
	switch(type) {
	case PT_VERTEX:
		g_vp[index + 3*offset] = pt.x;
		g_vp[index + 3*offset + 1] = pt.y;
		g_vp[index + 3*offset + 2] = pt.z;
		break;
	case PT_NORMAL:
		g_vn[index + 3*offset] = pt.x;
		g_vn[index + 3*offset + 1] = pt.y;
		g_vn[index + 3*offset + 2] = pt.z;
		break;
	case PT_TEXTURE:
		g_vt[index + 2*offset] = pt.x;
		g_vt[index + 2*offset + 1] = pt.y;
		break;
	default:
		break;
	}
}

void DisplaceTerrain::construct(bool drawOutline){

	construct(Vector2i (0,0), n, 2*height);
	//initLOD();
	updateQuadMesh(drawOutline);
	//updateMesh();
	//baseModelMatrix = glm::mat4(1.0);
	//baseModelMatrix[3] = glm::vec4(-(n*tile_size)/2, -3.0, -(n*tile_size)/2, 1.0);
	//modelMatrix = apply_model_mat * baseModelMatrix;
	//meshLoaded = true;
}

//construct terrain using diamond square algorithm
//INPUT:	coordinates of top left corner, and the width of the current square
void DisplaceTerrain::construct(Vector2i p00, int width, float tHeight){
	if(root != NULL){
		root->clear();
	}
	float average;
	DataPoint corners[] = { DataPoint(0,0,height),	//NW
						    DataPoint(n,0,height),	//NE
							DataPoint(0,n,height),	//SW
							DataPoint(n,n,height) };	//SE

	root = new QuadTree(0, BoundingBox(corners, n));
	//root->initCamPos(camPos);
	root->initSelectionList(n*n);

	//root->init();

	//root->insert(0, 0, height);
	//root->insert(n, 0, height);
	//root->insert(0, n, height);
	//root->insert(n, n, height);

	for(;width > 1; width/=2, tHeight*=powf(2,-reduction)){
		
		// DIAMOND STEP
		for(int y=0; y < n; y+=width){
			for(int x=0; x < n; x+=width){
				
				average = (map[y][x] + 
					map[y][x + width] + 
					map[y + width][x] + 
					map[y + width][x + width])/4;

				average += getPseudoRand(tHeight/2, -tHeight/2);

				map[y + width/2][x + width/2] = average;
				root->insert(x + width/2, y + width/2, average);
			} // end of x loop
		} // end of y loop


		//SQUARE STEP
		for(int y=0; y < n; y+=width/2){
			for(int x=(y+width/2)%width; x < n; x+=width){

				//calculate average from the diamond corners
				average = (map[(y-width/2 + n)%n][x] +		//above center
						map[y][(x-width/2 + n)%n] +			//left of center
						map[y][(x+width/2)%n] +				//right of center
						map[(y+width/2)%n][x])/4;			//below center

				average += getPseudoRand(tHeight/2, -tHeight/2);

				map[y][x] = average;
				root->insert(x,y,map[y][x]);

				if(y == 0) {
					map[n][x] = average;
					root->insert(x,n,map[n][x]);
				}
				if(x == 0) {
					map[y][n] = average;
					root->insert(n,y,map[y][n]);
				}

			}// end of x loop
		}// end of y loop
		
	} // main loop
	cout << "Generating Heightmap done" << endl;
}

void DisplaceTerrain::buildGradientMap(){
	for(int i = 0; i < n+1; i++){
		for(int j = 0; j < n+1; j++){
			gradient_map[i][j] = glm::vec2(getRandGradient());
		}
	}
}

/*
		  u ____________ v
		   /	       /
		  /		*	  /
		 /		     /
		/___________/
	   s	         t
*/
void DisplaceTerrain::constructPerlin2D(){
	gradient_map.resize(n+1);
	for(int m = 0; m < n+1; m++){
		gradient_map[m].resize(n+1);
	}
	buildGradientMap();

	map.resize(n+1);
	for(int m = 0; m < n+1; m++){
		map[m].resize(n+1);
	}


	vector<float> Q(4);
	float Sx, Sy, a, b;
	float sum = 0;
	
	
	for(int i = 0; i < n; i++){
		for(int j = 0; j < n; j++){
			vector<glm::vec2> gridGradients(4);
			vector<glm::vec2> gridToCenter(4);
			//for each square grid area, find random point inside square
			glm::vec2 center_point( getPseudoRand(1,0), getPseudoRand(1,0) );

			//get random gradients for each of the 4 surrounding grid points
			gridGradients[0] = gradient_map[i][j];
			gridGradients[1] = gradient_map[i+1][j];
			gridGradients[2] = gradient_map[i][j+1];
			gridGradients[3] = gradient_map[i+1][j+1];

			//get vectors pointing from each grid point to the chosen point inside square
			gridToCenter[0] = center_point - glm::vec2(0,0);
			gridToCenter[1] = center_point - glm::vec2(1,0);
			gridToCenter[2] = center_point - glm::vec2(0,1);
			gridToCenter[3] = center_point - glm::vec2(1,1);

			//dot product the two vectors
			Q[0] = glm::dot(gridGradients[0], -gridToCenter[0]);
			Q[1] = glm::dot(gridGradients[1], -gridToCenter[1]);
			Q[2] = glm::dot(gridGradients[2], -gridToCenter[2]);
			Q[3] = glm::dot(gridGradients[3], -gridToCenter[3]);

			//apply ease curve
			// f(x) = 3x^2 - 2x^3

			Sx = 3*powf(center_point.x, 2) 
				- 2*powf(center_point.x, 3);


			Sy = 3*powf(center_point.y, 2) 
				- 2*powf(center_point.y, 3);

			a = Q[0] + Sx*(Q[1] - Q[0]);
			b = Q[2] + Sx*(Q[3] - Q[2]);

			map[i][j] = a + Sy*(b - a);


		}
	}

	updateMesh();

}

void DisplaceTerrain::updateVertexArray(bool drawOutline){
	updateQuadMesh(drawOutline);
	superduper::updateVertexArray();
}

void DisplaceTerrain::updateCamProperties(Frustum &cam_view){
	camView = cam_view;
}

float DisplaceTerrain::getPseudoRand(float max, float min){
	if(!seedSet){
		cout << "Error! Random seed not specified! Exiting.." << endl;
		event_log << "Error! Random seed not specified! Exiting.." << endl;
		exit(-1);
	}
	else {
		//seed = logf(seed+1);
		//seed++;
	}
	
	float num = rand_height();

	//event_log << "for seed " << seed << ": " << num << endl;

	//float ans = fmod(rand(), max - min + 1) + min;

	return num*max + (1-num)*min;
}

glm::vec3 DisplaceTerrain::getRandGradient(){
	float x = getPseudoRand(1,-1);
	float y = getPseudoRand(1,-1);
	float z = getPseudoRand(1,-1);
	glm::vec3 gradient(x, y, z);
	normalize(gradient);
	return gradient;
}

int DisplaceTerrain::getNumTriangles(){
	return root->getNumSelected()*2;
}

float DisplaceTerrain::getCenterDistance(){
	//for(int i = 0; i < 4; i++){
	//	glm::vec3 sample = root->getBox()[i].getVec3();
	//	if((sample - camPos).length() < min_distance)
	//		min_distance = (sample - camPos).length();
	//}
	glm::vec3 sample(n/2, root->getDataPointAt(n/2,n/2), n/2);
	sample *= tile_size;
	return length(sample - *camView.origin);
}



void DisplaceTerrain::print(){
	for(int k = 0; k < n+1; k++){
		for(int l = 0; l < n+1; l++){
			std::cout << std::setprecision(2)<< map[k][l] <<"\t\t";
		}
		std::cout<< std::endl;
	}
}

