#include "DisplaceTerrain.hpp"
#include "event_logger.h"
#include <ctime>
#include <cstdlib>
#include <cmath>
#include <cstdarg>
#include <iostream>
#include <iomanip>

//math funcs
void normalize(glm::vec3 &vector){
	float mag = sqrtf((vector.x*vector.x)
					+ (vector.y*vector.y)
					+ (vector.z*vector.z));
	vector.x /= mag;
	vector.y /= mag;
	vector.z /= mag;
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

//Cross product of 2 vectors
//INPUT:	takes floats of 2 vectors and calculates cross product
//OUTPUT:	glm::vec3 object
glm::vec3 cross(float ax,float ay, float az, float bx, float by, float bz){
	glm::vec3 ans;
	ans.x = ay*bz - az*by;
	ans.y = az*bx - ax*bz;
	ans.z = ax*by - ay*bx; 
	normalize(ans);
	return ans;
}

//Cross product of 2 vectors
//INPUT:	Takes 2 glm::vec3 objects
//OUTPUT:	glm::vec3 object
glm::vec3 cross(glm::vec3 a, glm::vec3 b){
	glm::vec3 ans;
	ans.x = a.y*b.z - a.z*b.y;
	ans.y = a.z*b.x - a.x*b.z;
	ans.z = a.x*b.y - a.y*b.x; 
	normalize(ans);
	return ans;
}

DisplaceTerrain::DisplaceTerrain(int n, float height, float reduction, float tile_size) 
	: n(n), height(height), reduction(reduction), tile_size(tile_size), seed(-999),
	  seedSet(false){

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
	
}

DisplaceTerrain::~DisplaceTerrain(){
	//for(int i = 0; i < n+1; i++){
	//	delete[] map[i];
	//}
	//delete[] map;
	delete root;
}

//float avg(float num, ...){
//	va_list args;
//	float sum = 0;
//
//	va_start(args, num);
//	for(int i = 0; i < num; i++)
//		sum += va_arg(args, float);
//	va_end(args);
//	
//	return (sum/num);
//}


void DisplaceTerrain::setSeed(int seed){
	if(seed != this->seed){
		this->seed = seed;
		rand_height.seed(seed);
	}
	seedSet = true;
}

void DisplaceTerrain::construct(int n, float height, float reduction, float tile_size){
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
	construct();
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

	// n x n grid mesh has n*n squares, and (n+1)*(n+1) vertices
	//However, each square is composed of 2 triangles, and each
	//	triangle is composed of 3 vertices. So, g_vp will have 3x the 
	//	total number of triangles, which is always: n*n*2.
	//For each square, draw 2 triangles
	for(int i = 0; i < n; i++){
		for(int j = 0; j < n; j++){

			//Normal Calculation
			glm::vec3 topleft(j*tile_size,map[i][j],i*tile_size);
			glm::vec3 botleft(j*tile_size,map[i+1][j],(i+1)*tile_size);
			glm::vec3 topright((j+1)*tile_size,map[i][j+1],i*tile_size);
			glm::vec3 botright((j+1)*tile_size,map[i+1][j+1],(i+1)*tile_size);

			// triangle 1 normals: (topright - botleft) X (topleft - botleft)
			glm::vec3 normal1 = cross( (topright - botleft) , (topleft - botleft) );
			normalize(normal1);
			
			// triangle 2 normals: (botright - botleft) X (topright - botleft)
			glm::vec3 normal2 = cross( (botright - botleft) , (topright - botleft) );
			normalize(normal2);

			//***************
			//	TRIANGLE 1
			//***************
			//top left corner verts
			g_vp[(18*n*i)+(18*j)] = topleft.x;		//x
			g_vp[(18*n*i)+(18*j)+1] = topleft.y;		//y
			g_vp[(18*n*i)+(18*j)+2] = topleft.z;		//z

			//top left corner normals
			normals[(6*n*i)+(6*j)] = normal1;	


			//top left corner tex coords
			if (textureLoaded) {
				g_vt[(12*n*i)+(12*j)] = j*(1.0f/n);	
				g_vt[(12*n*i)+(12*j)+1] = i*(1.0f/n); 
			}

			//bottom left corner verts
			g_vp[(18*n*i)+(18*j)+3] = botleft.x;		//x
			g_vp[(18*n*i)+(18*j)+4] = botleft.y;		//y
			g_vp[(18*n*i)+(18*j)+5] = botleft.z;		//z

			//bottom left corner normals
			normals[(6*n*i)+(6*j)+1] = normal1;


			//bottom left corner tex coords
			if(textureLoaded) {
				g_vt[(12*n*i)+(12*j)+2] = j*(1.0f/n);	
				g_vt[(12*n*i)+(12*j)+3] = (i+1)*(1.0f/n);
			}

			//top right corner verts
			g_vp[(18*n*i)+(18*j)+6] = topright.x;		//x
			g_vp[(18*n*i)+(18*j)+7] = topright.y;		//y
			g_vp[(18*n*i)+(18*j)+8] = topright.z;		//z

			//top right corner normals
			normals[(6*n*i)+(6*j)+2] = normal1;


			//top right corner tex coords
			if(textureLoaded) {
				g_vt[(12*n*i)+(12*j)+4] = (j+1)*(1.0f/n);	
				g_vt[(12*n*i)+(12*j)+5] = i*(1.0f/n);
			}


			//***************
			//	TRIANGLE 2
			//***************
			//bottom left corner
			g_vp[(18*n*i)+(18*j)+9] = botleft.x;		//x
			g_vp[(18*n*i)+(18*j)+10] = botleft.y;		//y
			g_vp[(18*n*i)+(18*j)+11] = botleft.z;		//z

			//bottom left corner normals
			normals[(6*n*i)+(6*j)+3] = normal2;


			//bottom left corner tex coords
			if(textureLoaded) {
				g_vt[(12*n*i)+(12*j)+6] = j*(1.0f/n);	
				g_vt[(12*n*i)+(12*j)+7] = (i+1)*(1.0f/n);
			}


			//bottom right corner
			g_vp[(18*n*i)+(18*j)+12] = botright.x;		//x
			g_vp[(18*n*i)+(18*j)+13] = botright.y;		//y
			g_vp[(18*n*i)+(18*j)+14] = botright.z;		//z

			//bottom right corner normals
			normals[(6*n*i)+(6*j)+4] = normal2;


			//bottom right corner tex coords
			if(textureLoaded) {
				g_vt[(12*n*i)+(12*j)+8] = (j+1)*(1.0f/n);	
				g_vt[(12*n*i)+(12*j)+9] = (i+1)*(1.0f/n);
			}


			//top right corner
			g_vp[(18*n*i)+(18*j)+15] = topright.x;		//x
			g_vp[(18*n*i)+(18*j)+16] = topright.y;		//y
			g_vp[(18*n*i)+(18*j)+17] = topright.z;		//z

			//top right corner normals
			normals[(6*n*i)+(6*j)+5] = normal2;


			//top right corner tex coords
			if(textureLoaded) {
				g_vt[(12*n*i)+(12*j)+10] = (j+1)*(1.0f/n);	
				g_vt[(12*n*i)+(12*j)+11] = i*(1.0f/n);
			}

		}
	}
	glm::vec3 avgNormal;
	for(int i = 0; i < n; i++){
		for(int j = 0; j < n; j++){

			//***************
			//	Triangle 1
			//***************

			//top left
			avgNormal = getAvgNormal(normals,i,j,n);
			g_vn[(18*n*i)+(18*j)] = avgNormal.x;
			g_vn[(18*n*i)+(18*j)+1] = avgNormal.y;
			g_vn[(18*n*i)+(18*j)+2] = avgNormal.z;

			//bottom left
			avgNormal = getAvgNormal(normals,(i+1),j,n);
			g_vn[(18*n*i)+(18*j)+3] = avgNormal.x;
			g_vn[(18*n*i)+(18*j)+4] = avgNormal.y;
			g_vn[(18*n*i)+(18*j)+5] = avgNormal.z;

			//top right
			avgNormal = getAvgNormal(normals,i,(j+1),n);
			g_vn[(18*n*i)+(18*j)+6] = avgNormal.x;
			g_vn[(18*n*i)+(18*j)+7] = avgNormal.y;
			g_vn[(18*n*i)+(18*j)+8] = avgNormal.z;


			//***************
			//	Triangle 2
			//***************

			//bottom left
			avgNormal = getAvgNormal(normals,(i+1),j,n);
			g_vn[(18*n*i)+(18*j)+9] = avgNormal.x;
			g_vn[(18*n*i)+(18*j)+10] = avgNormal.y;
			g_vn[(18*n*i)+(18*j)+11] = avgNormal.z;

			//bottom right
			avgNormal = getAvgNormal(normals,(i+1),(j+1),n);
			g_vn[(18*n*i)+(18*j)+12] = avgNormal.x;
			g_vn[(18*n*i)+(18*j)+13] = avgNormal.y;
			g_vn[(18*n*i)+(18*j)+14] = avgNormal.z;

			//top right
			avgNormal = getAvgNormal(normals,i,(j+1),n);
			g_vn[(18*n*i)+(18*j)+15] = avgNormal.x;
			g_vn[(18*n*i)+(18*j)+16] = avgNormal.y;
			g_vn[(18*n*i)+(18*j)+17] = avgNormal.z;

		}
	}
	baseModelMatrix = glm::mat4(1.0);
	baseModelMatrix[3] = glm::vec4(-(n*tile_size)/2, -3.0, -(n*tile_size)/2, 1.0);
	modelMatrix = apply_model_mat * baseModelMatrix;
	//diffuse_reflectance = glm::vec3(0.7, 0.7, 0.3);
	meshLoaded = true;
}

void DisplaceTerrain::updateQuadMesh(){
}

void DisplaceTerrain::construct(){
	construct(Vector2i (0,0), n, 2*height);
	updateMesh();
}

//construct terrain using diamond square algorithm
//INPUT:	coordinates of top left corner, and the width of the current square
void DisplaceTerrain::construct(Vector2i p00, int width, float tHeight){

	float average;
	DataPoint corners[] = { DataPoint(0,0,map[0][0]),	//NW
						    DataPoint(n,0,map[0][n]),	//NE
							DataPoint(0,n,map[n][0]),	//SW
							DataPoint(n,n,map[n][n]) };	//SE

	root = new QuadTree(0, BoundingBox(corners, n+1));

	root->insert(0, 0, map[0][0]);
	root->insert(n, 0, map[0][n]);
	root->insert(0, n, map[n][0]);
	root->insert(n, n, map[n][n]);

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

glm::vec3 DisplaceTerrain::getRandGradient(){
	float x = getPseudoRand(1,-1);
	float y = getPseudoRand(1,-1);
	float z = getPseudoRand(1,-1);
	glm::vec3 gradient(x, y, z);
	normalize(gradient);
	return gradient;
}

void DisplaceTerrain::print(){
	for(int k = 0; k < n+1; k++){
		for(int l = 0; l < n+1; l++){
			std::cout << std::setprecision(2)<< map[k][l] <<"\t\t";
		}
		std::cout<< std::endl;
	}
}

