#include "DisplaceLine.hpp"
#include <glm/gtc/type_ptr.hpp>
#include <ctime>
#include <math.h>


//constructs a line segment that has been divided 
//iterations times by midpoint displacement
//to create a randomly generated line segment
//INPUT:	(int) number of iterations of midpoint displacement
//			(int) length of line
//			(mat4) model-to-world translation matrix to translate this line
DisplaceLine::DisplaceLine(int iterations, int length, glm::mat4 translation) 
		: numPoints(NULL), surface(NULL){
	this->iterations = iterations;
	this->length = length;
	this->modelMatrix = translation;
	numLineSegments = 1;
	srand(time(0));

}

int round(float num){
	float decimal = fmod(num, 1.0);
	int number = (int)num;
	if(decimal == 0)
		return (int)num;
	else if(decimal >= 0.5)
		return number + 1;
	else
		return number;
}

void DisplaceLine::initLine(){
	int iter = powf(2.0,iterations);
	int tempLength = iter+1;

	float stride = 0;

	tempPoints = std::vector<float>(tempLength);

	int midpoint = tempLength/2;
	float value = (rand() % (2*iter));
	if(midpoint != 0 && midpoint != tempPoints.size()-1){
		//if(tempPoints[round(midpoint)] == 0.0)
		tempPoints[midpoint] = value/iter;
	}
	int count = 0;
	stride = tempPoints[midpoint] - tempPoints[0];

	//extrapolate other points
	while(count < midpoint){
		tempPoints[count] = count*(stride/midpoint);
		count++;
	}
	count = midpoint+1;
	while(count < tempPoints.size()-1){
		tempPoints[count] = (tempPoints.size() - 1 - count)*(stride/midpoint);
		count++;
	}
	numLineSegments = 2;
}

//generates line
void DisplaceLine::generateLine(){
	
	if(numPoints == NULL){
		initLine();

		int iter = tempPoints.size()-1;
		int segmentLength = (iter + 1)/numLineSegments;
		int start = 0;
		int precisionFactor = iter;
		float H = 1.0;

		float rangeHigh = precisionFactor,
			rangeLow = 0;

		float value = 0, last_value = precisionFactor,
			highest_value = -1;

		//std::vector<float> tempPoints(segmentLength, 0.0);
		while(numLineSegments < iter) {

			//for each line segment, get random Y
			for(int j = 0; j < numLineSegments; j++){
				int midpoint = start + segmentLength/2;
				value = fmod(rand(), rangeHigh) + rangeLow + tempPoints[midpoint]*precisionFactor;
				if(midpoint != 0 && midpoint != tempPoints.size()-1){
					//if(tempPoints[round(midpoint)] == 0.0)
						tempPoints[midpoint] = value/precisionFactor;
				}
				start += segmentLength;
			}
			last_value = value;
			start = 0;
			rangeHigh /= powf(2.0, -H);
			rangeLow /= powf(2.0, -H);
			segmentLength /= 2;
			//rangeFactor += 5;
			//if((iterations+1)%2 != 0)
			//	numLineSegments++;
			numLineSegments*=2;
		}
		int sz = iter+1;
		float x = -length/2;
		numPoints = new float[sz*3];
		for(int j = 0; j < sz; j++){
			numPoints[j*3] = x;
			numPoints[j*3+1] = tempPoints[j];
			numPoints[j*3+2] = 0.0;
			x += (float)length/(sz-1);
		}
	}
	else {
		numLineSegments = 1;
		delete[] numPoints;
		numPoints = NULL;
		generateLine();
	}
	updateMesh();
}
void DisplaceLine::generateLine(int iterations, glm::mat4 translation){
	this->iterations = iterations;
	this->modelMatrix = translation;
	generateLine();
}

void DisplaceLine::updateMesh(){
	g_vp.resize(tempPoints.size()*3);
	g_vn.resize(tempPoints.size()*3);

	for(int i = 0; i < size_points; i++){
		g_vp[3*i] = numPoints[3*i];
		g_vp[3*i+1] = numPoints[3*i+1];
		g_vp[3*i+2] = numPoints[3*i+2];

		g_vn[3*i] = numPoints[3*i];
		g_vn[3*i+1] = numPoints[3*i+1];
		g_vn[3*i+2] = numPoints[3*i+2];
	}
	diffuse_reflectance = glm::vec3(1.0);
	meshLoaded = true;
}

int DisplaceLine::getSize(){
	return pow(2.0,iterations)+1;
}

float * DisplaceLine::getTransformation(){
	return glm::value_ptr(modelMatrix);
}


DisplaceLine::~DisplaceLine(){
	delete[] numPoints;
	delete[] surface;
}