#ifndef __DISPLACE_LINE_H_
#define __DISPLACE_LINE_H_
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <vector>
#include "Mesh.hpp"

class DisplaceLine : public Mesh{
public:
	//constructs a line segment that will be divided 
	//displacementPoints times by midpoint displacement
	//to create a randomly generated line segment
	//INPUT:	(int) number of iterations of midpoint displacement
	//			(int) length of line
	//			(mat4) model-to-world translation matrix to translate this line
	DisplaceLine::DisplaceLine(int displacementPoints, int length, glm::mat4 translation);

	DisplaceLine::~DisplaceLine();
	

	//generates line
	void DisplaceLine::generateLine();
	void DisplaceLine::generateLine(int iterations, glm::mat4 translation);

	//	Getters
	int DisplaceLine::getSize();
	float * DisplaceLine::getTransformation();
	

	//	Extras
	friend int round(float num);

	

private:
	void DisplaceLine::initLine();

	//Overloaded updateMesh() method
	void DisplaceLine::updateMesh();

	int iterations, length, numLineSegments, size;
	//glm::mat4 translation;
	float *numPoints;
	int size_points;
	float *surface;
	std::vector<float> tempPoints;

};

#endif