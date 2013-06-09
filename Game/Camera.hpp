#ifndef _CAMERA_HPP_
#define _CAMERA_HPP_
#include <SFML/Graphics.hpp>
#include <GL/glew.h>
#include <GL/freeglut.h>
#include <glm/glm.hpp>
#include "Quaternion.hpp"

using namespace sf;

class Camera{
public:
	enum cameraType {c_FirstPerson, c_Flying, c_ThirdPerson, c_Orbiting} m_camera_type; //camera types

	static Camera *Camera::getInstance();

	void Camera::initCamera();

	void Camera::LookAt();
	
	void Camera::updateCamera(float angleX, float angleY);
	void Camera::updateRightVec();
	void Camera::updateViewMatrix();

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

	Camera::~Camera();

	//Math functions
	void Camera::normalize(glm::vec3 &vector);
	float Camera::dot(glm::vec3 &a, glm::vec3 &b);
	glm::vec3 Camera::crossProduct(float ax,float ay, float az, 
		float bx, float by, float bz);
	glm::vec3 Camera::crossProduct(glm::vec3 a, glm::vec3 b);
	float Camera::magnitude(glm::vec3 &vector);

	


protected:	
	glm::vec3 m_target, m_camOrigin;
	glm::vec3 lookAtVec, rightVec, upVec;
	glm::vec3 m_dir;

	//perspective variables
	struct ProjBox {
		float left, right, bottom, top;
		float fovy;
		float aspect;
		float zNear;
		float zFar;
		float range;
	};

	ProjBox *orthoBox, *persBox;

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
