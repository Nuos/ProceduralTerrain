#ifndef __QUATERNION_TOOL_H_
#define __QUATERNION_TOOL_H_
#define ROUNDING_TOLERANCE 0.00001f
#define RAD2DEG (180/PI)
#include <math.h>

namespace eng {

	class quat {
	public:
		quat::quat() : x(0), y(0), z(0), w(0){}
		quat::quat(float x, float y, float z, float w)
			  : x(x), y(y), z(z), w(w) {}
		quat::quat(glm::vec3& axis, float angle) {
			buildFromAxis(axis, angle);
		}

		//copy constructor
		quat::quat(const quat& other){
			*this = other;
		}
		quat::~quat(){}

		quat& quat::operator=(const quat& rhs){
			w = rhs.w;
			x = rhs.x;
			y = rhs.y;
			z = rhs.z;
			return *this;
		}

		quat& quat::operator*(const quat& rhs){
			float qw = w,
				  qx = x,
				  qy = y,
				  qz = z;
			w = qw*rhs.w - qx*rhs.x - qy*rhs.y - qz*rhs.z;
			x = qw*rhs.x + qx*rhs.w - qy*rhs.z + qz*rhs.y;
			y = qw*rhs.y + qx*rhs.z + qy*rhs.w - qz*rhs.x;
			z = qw*rhs.z - qx*rhs.y + qy*rhs.x + qz*rhs.w;
			return *this;
		}

		//apply a Q-rotation to a vector
		//	multiply a vector by q.conjugate, and then q
		// v' = q * v * q.conjugate()
		glm::vec3& quat::operator*(const glm::vec3& rhs){
			glm::vec3 other(rhs.x, rhs.y, rhs.z);
			float mag = sqrtf(other.x*other.x + other.y*other.y + other.z*other.z);
			other.x /= mag;
			other.y /= mag;
			other.z /= mag;

			quat qVec(other.x, other.y, other.z, 0.0), qTemp;

			qTemp = qVec * conjugate();
			qTemp = *this * qTemp;

			return glm::vec3(qTemp.x, qTemp.y, qTemp.z);
		}

		quat& quat::operator*=(const quat& rhs){
			*this = *this * rhs;
			return *this;
		}

		void quat::normalize(){
			float magnitude = sqrtf(w*w + x*x + y*y + z*z);
			if(fabs(magnitude - 1) > ROUNDING_TOLERANCE 
				&& fabs(magnitude) > ROUNDING_TOLERANCE){
				w /= magnitude;
				x /= magnitude;
				y /= magnitude;
				z /= magnitude;
			}
		}

		quat quat::conjugate(){
			return quat(-x, -y, -z, w);
		}

		//Angle in radians
		void quat::buildFromAxis(glm::vec3& axis, float angle){
			angle *= DEG2RAD;
			angle *= 0.5;
			float sinAngle = sinf(angle);
			quat qn(axis.x, axis.y, axis.z , 0.0);
			qn.normalize();

			x = (qn.x * sinAngle);
			y = (qn.y * sinAngle);
			z = (qn.z * sinAngle);
			w = cosf(angle);
			normalize();
		}

		//Creates quaternion from Euler angles
		//INPUT:	rotation about X axis in degrees,
		//			rotation about Y axis in degrees,
		//			rotation about Z axis in degrees
		void quat::buildFromEuler(float xRot, float yRot, float zRot){
			float pitch = xRot*DEG2RAD / 2.0;
			float yaw = yRot*DEG2RAD / 2.0;
			float roll = zRot*DEG2RAD / 2.0;

			float sinp = sinf(pitch),
			siny = sinf(yaw),
			sinr = sinf(roll),
			cosp = cosf(pitch),
			cosy = cosf(yaw),
			cosr = cosf(roll);

			x = sinr * cosp * cosy - cosr * sinp * siny;
			y = cosr * sinp * cosy + sinr * cosp * siny;
			z = cosr * cosp * siny - sinr * sinp * cosy;
			w = cosr * cosp * cosy + sinr * sinp * siny;

			normalize();
		}

		void quat::buildFromMatrix(glm::mat4 m){
			
			float s = 0.0f;
			float q[4] = {0.0f};
			float trace = m[0][0] + m[1][1] + m[2][2];

			if (trace > 0.0f)
			{
				s = sqrtf(trace + 1.0f);
				q[3] = s * 0.5f;
				s = 0.5f / s;
				q[0] = (m[1][2] - m[2][1]) * s;
				q[1] = (m[2][0] - m[0][2]) * s;
				q[2] = (m[0][1] - m[1][0]) * s;
			}
			else
			{
				int nxt[3] = {1, 2, 0};
				int i = 0, j = 0, k = 0;

				if (m[1][1] > m[0][0])
					i = 1;

				if (m[2][2] > m[i][i])
					i = 2;

				j = nxt[i];
				k = nxt[j];
				s = sqrtf((m[i][i] - (m[j][j] + m[k][k])) + 1.0f);

				q[i] = s * 0.5f;
				s = 0.5f / s;
				q[3] = (m[j][k] - m[k][j]) * s;
				q[j] = (m[i][j] + m[j][i]) * s;
				q[k] = (m[i][k] + m[k][i]) * s;
			}

			x = q[0], y = q[1], z = q[2], w = q[3];
			normalize();
		}

		glm::mat4 quat::getMatrix(){
			float x2 = x * x;
			float y2 = y * y;
			float z2 = z * z;
			float xy = x * y;
			float xz = x * z;
			float yz = y * z;
			float wx = w * x;
			float wy = w * y;
			float wz = w * z;

			return glm::mat4( 1.0f - 2.0f * (y2 + z2), 2.0f * (xy - wz), 2.0f * (xz + wy), 0.0f,
				2.0f * (xy + wz), 1.0f - 2.0f * (x2 + z2), 2.0f * (yz - wx), 0.0f,
				2.0f * (xz - wy), 2.0f * (yz + wx), 1.0f - 2.0f * (x2 + y2), 0.0f,
				0.0f, 0.0f, 0.0f, 1.0f);
		}

		//sets angle in degrees
		void quat::getAxisAngle(glm::vec3 *vec, float *angle){
			float mag = sqrtf(x*x + y*y + z*z);
			vec->x = x / mag;
			vec->y = y / mag;
			vec->z = z / mag;
			*angle = acosf(w) * 2.0;
			*angle *= RAD2DEG;
		}

		float quat::getMagnitude(){
			return sqrt(x*x + y*y + z*z + w*w);
		}

		float w, x, y, z;
		//float magnitude;
	};



}
#endif
