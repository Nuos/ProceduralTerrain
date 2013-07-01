#ifndef _CAMERA_HPP_
#define _CAMERA_HPP_
#define PI 3.1415
#define DEG2RAD (PI/180)
#define RAD2DEG (180/PI)
#include <SFML/Graphics.hpp>
#include <GL/glew.h>
#include <GL/freeglut.h>
#include <glm/glm.hpp>
#include "Quaternion.hpp"

using namespace sf;

//Math function prototypes
void normalize(glm::vec3 &vector);
glm::vec3 getNormalized(glm::vec3 vector);
float dot(glm::vec3 &a, glm::vec3 &b);
glm::vec3 crossProduct(float ax,float ay, float az, 
					   float bx, float by, float bz);
glm::vec3 crossProduct(glm::vec3 a, glm::vec3 b);
float magnitude(glm::vec3 &vector);

struct Frustum {

	enum {
		TOPPLANE,
		BOTPLANE,
		LEFTPLANE,
		RIGHTPLANE,
		NEARPLANE,
		FARPLANE,
		NUMPLANES
	};

	struct Plane {
		Plane::Plane(){ *this = 0; }
		Plane::Plane(float nx, float ny, float nz, float bx, float by, float bz)
			  : normal(glm::vec3(nx,ny,nz)), basePt(glm::vec3(bx,by,bz)){}
		Plane::Plane(glm::vec3 normal_vector, glm::vec3 plane_base_point)
			: normal(normal_vector), basePt(plane_base_point){}
		
		Plane::Plane(const Plane& other){ *this = other; }

		const Plane& Plane::operator=(const Plane& other){
			normal = other.normal;
			basePt = other.basePt;
			return *this;
		}

		const Plane& Plane::operator=(const int num){
			normal = glm::vec3(0);
			basePt = glm::vec3(0);
			return *this;
		}

		//Finds the signed distance between a point and this plane
		//RETURNS: positive value if pt is the same side as normal
		float Plane::signedDistance(glm::vec3 pt){
			//float d = -dot(normal, basePt);
			//return						((dot(normal,pt) + d) /
			//		  sqrtf(normal.x*normal.x + normal.y*normal.y + normal.z*normal.z));
			return dot(pt-basePt, normal);
		}

		glm::vec3 normal;
		glm::vec3 basePt;
	};
public:
	Frustum::Frustum() : init(false) { }
	//Frustum::Frustum(Camera *cam, float fovy, float aspect, float zNear, float zFar)
	//	: origin(&cam->getCamOrigin()), forwardVec(cam->getTarget()), upVec(cam->getUpVec()),
	//	  fovy(fovy), aspect(aspect), zNear(zNear), zFar(zFar),
	//	  yNear(2*tanf(fovy/2)*zNear), xNear(aspect*yNear), init(true){ }

	Frustum::Frustum(const Frustum& other){ *this = other; }

	Frustum& Frustum::operator=(const Frustum& other){
		fovy = other.fovy;
		aspect = other.aspect;
		zNear = other.zNear;
		zFar = other.zFar;

		xNear = other.xNear;
		xFar = other.xFar;
		yNear = other.yNear;
		yFar = other.yFar;
		origin = other.origin;

		origin = other.origin;
		forwardVec = other.forwardVec;
		upVec = other.upVec;
		rightVec = other.rightVec;

		for(int s = 0; s < NUMPLANES; s++){
			planes[s] = other.planes[s];
		}

		return *this;
	}

	//calculates missing plane values
	void Frustum::updateFrustum(){
		yNear = tanf(fovy/2)*zNear;
		xNear = aspect * yNear;
		yFar = tanf(fovy/2)*zFar;
		xFar = aspect * yFar;

		planes[NEARPLANE].basePt = *origin + (*forwardVec)*zNear;
		planes[NEARPLANE].normal = getNormalized(*forwardVec);

		planes[FARPLANE].basePt = *origin + (*forwardVec)*zFar;
		planes[FARPLANE].normal = getNormalized(-(*forwardVec));

		planes[TOPPLANE].basePt = *origin;
		planes[TOPPLANE].normal = crossProduct(
			getNormalized(planes[FARPLANE].basePt + (*upVec)*yFar),
			*rightVec
			);
		normalize(planes[TOPPLANE].normal);

		planes[BOTPLANE].basePt = *origin;
		planes[BOTPLANE].normal = crossProduct(
			*rightVec,
			getNormalized(planes[FARPLANE].basePt - (*upVec)*yFar)
			);
		normalize(planes[BOTPLANE].normal);

		planes[RIGHTPLANE].basePt = *origin;
		planes[RIGHTPLANE].normal = crossProduct(
			*upVec,
			getNormalized(planes[FARPLANE].basePt + (*rightVec)*xFar)
			);
		normalize(planes[RIGHTPLANE].normal);

		planes[LEFTPLANE].basePt = *origin;
		planes[LEFTPLANE].normal = crossProduct(
			getNormalized(planes[FARPLANE].basePt - (*rightVec)*xFar),
			*upVec
			);
		normalize(planes[LEFTPLANE].normal);
	}

	//Finds the signed distance between pt and each plane
	//	if pt lies on the side of the plane opposite where the 
	//	normal points, it's outside the frustum
	//bool Frustum::pointInFrustum(glm::vec3 pt) {
	//	for(int t = 0; t < 6; t++){
	//		if(planes[t].signedDistance(pt) < 0)
	//			return false;
	//	}
	//	return true;
	//}

	float fovy, aspect, zNear, zFar,
		xNear, yNear, xFar, yFar;
	Plane planes[6];
	glm::vec3 *origin, *forwardVec, *upVec, *rightVec;
	bool init;
};

class Camera{
public:
	enum cameraType {c_FirstPerson, c_Flying, c_ThirdPerson, c_Orbiting} m_camera_type; //camera types

	static Camera *Camera::getInstance();

	void Camera::initCamera();

	void Camera::LookAt();
	
	void Camera::updateCamera(float angleX, float angleY);
	void Camera::updateRightVec();
	void Camera::updateViewMatrix();
	void Camera::updateFrustum();

	//modify cam orientation
	void Camera::rotateX(float angle);
	void Camera::rotateY(float angle);
	void Camera::rotateZ(float angle);

	//move camera
	void Camera::move(float deltaX, float deltaY, float deltaZ);


	//Setters
	void Camera::setTarget(float lookAtX, float lookAtY, float lookAtZ);
	void Camera::setCamOrigin(float camX, float camY, float camZ);
	void Camera::setUpVec(float upX, float upY, float upZ);
	void Camera::setLookAtVec();
	void Camera::enableZRoll();
	void Camera::disableZRoll();
	void Camera::setPerspective();
	void Camera::setPerspective(float fovy, float aspect, float zNear, float zFar);
	void Camera::setOrtho();
	void Camera::setOrtho(float left, float right, float bottom, float top,
								float zNear, float zFar);
	void Camera::setZoom(float zoom_distance);
	
	int Camera::checkInits();

	//Getters
	glm::vec3 Camera::getTarget();
	glm::vec3 Camera::getCamOrigin();
	glm::vec3 Camera::getDir();
	glm::vec3 Camera::getUpVec();
	glm::vec3 Camera::getRightVec();
	float *Camera::getViewMatrix();
	glm::mat4 Camera::getViewMat4();
	float *Camera::getProjMatrix();
	glm::mat4 Camera::getProjMat4();
	glm::mat4 Camera::getCamTransformMatrix();
	glm::vec3 Camera::getEyeSpacePos();
	Frustum Camera::getFrustum();

	Camera::~Camera();



	


protected:	
	glm::vec3 m_target, m_camOrigin;
	glm::vec3 lookAtVec, rightVec, upVec;
	glm::vec3 m_dir;

	//perspective variables
	//struct ProjBox {
	//	float left, right, bottom, top;
	//	float fovy;
	//	float aspect;
	//	float zNear;
	//	float zFar;
	//	float range;
	//};

	Frustum *orthoBox, *persBox;

	bool z_roll, isPerspective;

private:
	Camera::Camera();
	Camera::Camera(const Camera&);				//dont implement copy constructor for singleton
	Camera& Camera::operator=(const Camera&);	//dont implement assignment operator for singleton
	Camera::Camera(glm::vec3 target, glm::vec3 camOrigin);
	Camera::Camera(float originX, float originY, float originZ,
		float camOriginX, float camOriginY, float camOriginZ);

	

	float thirdPersonZoom;

	float lookAtLength;

	glm::mat4 mat_View, mat_Proj, m_R, m_T;
	glm::mat4 mat_rotX, mat_rotY, mat_rotZ;
	float angleX, angleY, angleZ;
	static Camera *instance;
	static int initCount;

	eng::quat orientation;

	float lastAngleX, accumAngleX,
		  lastAngleY, accumAngleY;
	float m_pitch;
};




#endif
