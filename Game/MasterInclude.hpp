#ifndef __MASTER_INCLUDE_H_
#define __MASTER_INCLUDE_H_
#define PI 3.1415
#define DEG2RAD (PI/180)
#include <SFML\System.hpp>
#include <SFML\Graphics.hpp>
#include <SFML\Window\Event.hpp>
#include <GL/glew.h>
//#include <GL/glext.h>
//#include <GL/freeglut.h>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <ctime>
#include <vector>
#include <iomanip>
#include <algorithm>
#include "Camera.hpp"
#include "MeshList.hpp"
#include "Shader.hpp"
#include "ShaderList.hpp"
#include "DisplaceTerrain.hpp"
#include "BitmapFont.hpp"

const string TextureDirectory = "../../../../../CODING/OpenGL/Textures/";
const string MeshDirectory = "..//..//..//..//..//CODING//OpenGL//Meshes//Blender exports//";



#endif