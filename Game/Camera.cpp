#include "Camera.hpp"
#include <SFML/Graphics.hpp>
#include <GL/glew.h>
#include <GL/freeglut.h>
#include <cassert>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <algorithm>
#include <iostream>

using namespace sf;
using namespace std;

Camera *Camera::instance = NULL;
int Camera::initCount = 0;

//get only one instance
Camera *Camera::getInstance(){
	if(instance == NULL)
		instance = new Camera();
	return instance;
}

//Constructors
Camera::Camera(){ 
	initCount++;
	initCamera();
}
Camera::Camera(glm::vec3 target, glm::vec3 camOrigin){
	this->m_target = target;
	this->lookAtVec = target - camOrigin;
	m_camOrigin = camOrigin;
	initCamera();
}
Camera::Camera(float lookAtX, float lookAtY, float lookAtZ, 
			   float camOriginX, float camOriginY, float camOriginZ){
	this->m_target = glm::vec3(lookAtX, lookAtY, lookAtZ);
	this->m_camOrigin = glm::vec3(camOriginX, camOriginY, camOriginZ);
	initCamera();
}

Camera::~Camera(){
	delete orthoBox;
	delete persBox;
	delete instance;
}

void Camera::initCamera(){
	lastAngleX = 0;
	accumAngleX = 0;
	lastAngleY = 0;
	accumAngleY = 0;
	orientation = eng::quat(0,0,0,1);
	lookAtVec = m_target - m_camOrigin;
	z_roll = false;
	isPerspective = false;
	this->m_camera_type = c_FirstPerson;
	thirdPersonZoom = 0;
	orthoBox = new Frustum();
	persBox = new Frustum();
	mat_View = glm::mat4(1.0);
	mat_Proj = glm::mat4(1.0);
	mat_rotX = glm::mat4(1.0);
	mat_rotY = glm::mat4(1.0);
	mat_rotZ = glm::mat4(1.0);
	m_R = glm::mat4(1.0);
	m_T = glm::mat4(1.0);

}

//************************************************
// Camera property changing functions
//************************************************

//private: Lookat function
//updates view matrix
//INPUT: eyeX, eyeY, eyeZ,
//		 targetX, targetY, targetZ,
//		 upX, upY, upZ
void Camera::LookAt()
{
	//world space coordinates
	//glm::vec3 target = glm::vec3(targetX, targetY, targetZ);
	//glm::vec3 camPos = glm::vec3(camPosX,camPosY,camPosZ);
	//glm::vec3 up = glm::vec3(upX,upY,upZ);

	setLookAtVec();

	rightVec = crossProduct(lookAtVec, upVec);
	normalize(rightVec);

	upVec = crossProduct(rightVec, lookAtVec);
	normalize(upVec);

	//if(z_roll)
		
	
	//set up R matrix - i.e. orientation
	//glm::mat4 m_R(
	//	right.x, upVec.x, -lookAt.x, 0.0,
	//	right.y, upVec.y, -lookAt.y, 0.0,
	//	right.z, upVec.z, -lookAt.z, 0.0,
	//	-zoom.x, -zoom.y, -zoom.z, 1.0);
	
	m_R[0][0] = rightVec.x;
	m_R[0][1] = upVec.x;
	m_R[0][2] = -lookAtVec.x;
	m_R[0][3] = 0;

	m_R[1][0] = rightVec.y;
	m_R[1][1] = upVec.y;
	m_R[1][2] = -lookAtVec.y;
	m_R[1][3] = 0;

	m_R[2][0] = rightVec.z;
	m_R[2][1] = upVec.z;
	m_R[2][2] = -lookAtVec.z;
	m_R[2][3] = 0;

	
	//For Third person view
	//m_R[3][2] = -thirdPersonZoom;

	m_R =  mat_rotZ * (mat_rotX * (mat_rotY * m_R));


	//set up T matrix - i.e. position
	//translation = {
	//	1,      0,      0,     0,    //first column
	//	0,      1,      0,     0,    //second column
	//	0,      0,      1,     0,    //third column
	//	-eye.x, -eye.y, -eye.z,  1   //fourth column
	//}
	m_T[3][0] = -m_camOrigin.x;
	m_T[3][1] = -m_camOrigin.y;
	m_T[3][2] = -m_camOrigin.z;

	//View matrix
	//{
	//	right.x, upVec.x, -lookAt.x, 0.0,	//first column
	//	right.y, upVec.y, -lookAt.y, 0.0,	//second column
	//	right.z, upVec.z, -lookAt.z, 0.0,	//third column
	//	-m_target.x, -m_target.y, -m_target.z,  1 }		//fourth column
	mat_View = m_R * m_T;

	m_pitch = asinf(mat_View[1][2]);
	//orientation.buildFromMatrix(mat_View);

	mat_Proj = glm::mat4(1.0);

	if(isPerspective){
		mat_Proj[0][0] = (2 * persBox->zNear) / 
			(persBox->yNear * persBox->aspect + persBox->yNear * persBox->aspect);
		mat_Proj[1][1] = persBox->zNear/persBox->yNear;
		mat_Proj[2][2] = -(persBox->zFar + persBox->zNear) / (persBox->zFar - persBox->zNear);
		mat_Proj[2][3] = -1;
		mat_Proj[3][2] = -(2 * persBox->zFar * persBox->zNear) / (persBox->zFar - persBox->zNear);
		mat_Proj[3][3] = 0;
		persBox->init = true;
	} else {
		//mat_Proj[0][0] = 2 / (orthoBox->right - orthoBox->left);
		//mat_Proj[1][1] = 2 / (orthoBox->top - orthoBox->bottom);
		//mat_Proj[2][2] = -2 / (orthoBox->zFar - orthoBox->zNear);
		//mat_Proj[3][0] = -(orthoBox->right + orthoBox->left) / (orthoBox->right - orthoBox->left);
		//mat_Proj[3][1] = -(orthoBox->top + orthoBox->bottom) / (orthoBox->top - orthoBox->bottom);
		//mat_Proj[3][2] = -(orthoBox->zFar + orthoBox->zNear) / (orthoBox->zFar - orthoBox->zNear);
		//mat_Proj[3][3] = 1;
		//orthoBox->init = true;
	}
	//updateViewMatrix();
}

void Camera::updateViewMatrix(){
	mat_View = orientation.getMatrix();
	orientation = eng::quat(0,0,0,1);
	lookAtVec = -glm::vec3(mat_View[0][2], mat_View[1][2], mat_View[2][2]);
	m_dir = lookAtVec;
	rightVec = glm::vec3(mat_View[0][0], mat_View[1][0], mat_View[2][0]);
	upVec = glm::vec3(mat_View[0][1], mat_View[1][1], mat_View[2][1]);

	mat_View[3][0] = -glm::dot(rightVec, m_camOrigin);
	mat_View[3][1] = -glm::dot(upVec, m_camOrigin);
	mat_View[3][2] = -glm::dot(-lookAtVec, m_camOrigin);
}

void Camera::updateCamera(float angleX, float angleY){

	accumAngleY += angleY - lastAngleY;
	glm::vec3 y_axis;
	y_axis = glm::vec3(0,1,0);
	eng::quat rotY(y_axis, -accumAngleY);
	orientation = rotY * orientation;
	orientation.normalize();
	lastAngleY = angleY;

	accumAngleX += angleX - lastAngleX;
	eng::quat rotX(glm::vec3(1,0,0), accumAngleX);
	orientation = rotX * orientation;
	orientation.normalize();
	lastAngleX = angleX;

	updateViewMatrix();
}

void Camera::updateRightVec(){
	rightVec = crossProduct(lookAtVec, upVec);
}


//angle in degrees
//rotate about axis according to Left Hand Rule
void Camera::rotateX(float angle){
	
}

//angle in degrees
//rotate about axis according to Left Hand Rule
void Camera::rotateY(float angle){

}

//angle in degrees
void Camera::rotateZ(float angle){
	//angleZ = angle;

	glm::mat4 rotZ(1.0);
	if(z_roll) {
		rotZ[0][0] = cosf(angle);
		rotZ[0][1] = sinf(angle);
		rotZ[1][0] = -sinf(angle);
		rotZ[1][1] = cosf(angle);
		mat_rotZ = rotZ;
	}
}

void Camera::move(float deltaX, float deltaY, float deltaZ){
	if(m_camera_type == c_Flying){
			if(deltaZ != 0.0f){
				m_camOrigin.x += deltaZ * m_dir.x;
				m_camOrigin.y += deltaZ * m_dir.y;
				m_camOrigin.z += deltaZ * m_dir.z; 
				m_target.x += deltaZ * m_dir.x;
				m_target.y += deltaZ * m_dir.y;
				m_target.z += deltaZ * m_dir.z;
			}
			if(deltaX != 0.0f){ 
				m_camOrigin.x += deltaX * rightVec.x;
				m_camOrigin.y += deltaX * rightVec.y;
				m_camOrigin.z += deltaX * rightVec.z; 
				m_target.x += deltaX * rightVec.x;
				m_target.y += deltaX * rightVec.y;
				m_target.z += deltaX * rightVec.z;
			}
			if(deltaY != 0.0f){ 
				m_camOrigin.x += deltaY * upVec.x;
				m_camOrigin.y += deltaY * upVec.y;
				m_camOrigin.z += deltaY * upVec.z; 
				m_target.x += deltaY * upVec.x;
				m_target.y += deltaY * upVec.y;
				m_target.z += deltaY * upVec.z;
			}
		}
}


//************************************************
// Set Functions
//************************************************
void Camera::setTarget(float lookAtX, float lookAtY, float lookAtZ){
	this->m_target.x = lookAtX;
	this->m_target.y = lookAtY;
	this->m_target.z = lookAtZ;
	//lookAtVec = m_target - m_camOrigin;
	//LookAt();
}

void Camera::setCamOrigin(float camX, float camY, float camZ){
	this->m_camOrigin.x = camX;
	this->m_camOrigin.y = camY;
	this->m_camOrigin.z = camZ;
}

void Camera::setUpVec(float upX, float upY, float upZ){
	this->upVec.x = upX;
	this->upVec.y = upY;
	this->upVec.z = upZ;
}

void Camera::setLookAtVec(){
	lookAtVec = m_target - m_camOrigin;
	normalize(lookAtVec);
	//lookAtLength = magnitude(lookAtVec);
}

void Camera::enableZRoll(){
	this->z_roll = true;
}

void Camera::disableZRoll(){
	this->z_roll = false;
}

void Camera::setPerspective(){
	isPerspective = true;
}
void Camera::setPerspective(float fovy, float aspect, float zNear, float zFar){
	persBox->fovy = fovy;
	persBox->aspect = aspect;
	persBox->zNear = zNear;
	persBox->zFar = zFar;
	persBox->origin = &m_camOrigin;
	persBox->forwardVec = &lookAtVec;
	persBox->upVec = &upVec;
	persBox->rightVec = &rightVec;
	persBox->updateFrustum();
	isPerspective = true;
}

void Camera::setOrtho(){
	isPerspective = false;
}
void Camera::setOrtho(float left, float right, 
							float bottom, float top,
							float zNear, float zFar){
	//orthoBox->left = left;
	//orthoBox->right = right;
	//orthoBox->bottom = bottom;
	//orthoBox->top = top;
	//orthoBox->zNear = zNear;
	//orthoBox->zFar = zFar;
	//isPerspective = false;
}

void Camera::setZoom(float zoom_distance){
	thirdPersonZoom = zoom_distance;
}

int Camera::checkInits(){
	return initCount;
}


//************************************************
// Get Functions
//************************************************
glm::vec3 Camera::getTarget(){
	return -lookAtVec;
}
glm::vec3 Camera::getCamOrigin(){
	return this->m_camOrigin;
}
glm::vec3 Camera::getDir(){
	return m_dir;
}
glm::vec3 Camera::getUpVec(){
	return upVec;
}
glm::vec3 Camera::getRightVec(){
	return rightVec;
}
float * Camera::getViewMatrix(){
	return glm::value_ptr(mat_View);
}
glm::mat4 Camera::getViewMat4(){
	return mat_View;
}
float * Camera::getProjMatrix(){
	return glm::value_ptr(mat_Proj);
}
glm::mat4 Camera::getProjMat4(){
	return mat_Proj;
}
glm::mat4 Camera::getCamTransformMatrix(){
	return mat_View._inverse();
}
glm::vec3 Camera::getEyeSpacePos(){
	return glm::vec3(mat_View * glm::vec4(m_camOrigin, 1.0));
}

Frustum Camera::getFrustum(){
	if(isPerspective){
		persBox->updateFrustum();
		return *persBox;
	}
	else {
		orthoBox->updateFrustum();
		return *orthoBox;
	}
}



//Normalizes a vector
//INPUT:	a reference to a glm::vec3-type object
void normalize(glm::vec3 &vector){
	float m = sqrtf((vector.x * vector.x) 
				+ (vector.y * vector.y)
				+ (vector.z * vector.z));
	vector.x /= m;
	vector.y /= m;
	vector.z /= m;
}

glm::vec3 getNormalized(glm::vec3 vector){
	normalize(vector);
	return vector;
}

//Dot product of 2 vectors
//INPUT:	Two references to SFML glm::vec3-type objects
//OUTPUT:	floating point answer
float dot(glm::vec3 &a, glm::vec3 &b)
{
    return (a.x * b.x) + (a.y * b.y) + (a.z * b.z);
}

//Cross product of 2 vectors
//INPUT:	takes floats of 2 vectors and calculates cross product
//OUTPUT:	SFML glm::vec3 object
glm::vec3 crossProduct(float ax,float ay, float az, float bx, float by, float bz){
	glm::vec3 ans;
	ans.x = ay*bz - az*by;
	ans.y = az*bx - ax*bz;
	ans.z = ax*by - ay*bx; 
	return ans;
}

//Cross product of 2 vectors
//INPUT:	Takes 2 SFML Vector3 objects
//OUTPUT:	SFML glm::vec3 object
glm::vec3 crossProduct(glm::vec3 a, glm::vec3 b){
	glm::vec3 ans;
	ans.x = a.y*b.z - a.z*b.y;
	ans.y = a.z*b.x - a.x*b.z;
	ans.z = a.x*b.y - a.y*b.x; 
	return ans;
}

//Magnitude of a vector
//INPUT:	A Vector
//OUTPUT:	Magnitude of Vector as a float
float magnitude(glm::vec3 &vector){
	return sqrtf(vector.x*vector.x + vector.y*vector.y + vector.z*vector.z);
}