#include "MasterInclude.hpp"
#include "MarchingCubesLUT.hpp"
#include <FTGL/ftgl.h>

using namespace sf;
//using namespace std;

ofstream event_log;

extern const string TextureDirectory;
extern const string MeshDirectory;

time_t simpleTime;
struct tm *realTime;

clock_t oldTime;
unsigned int fpsCount = 0, frameCount = 0;
std::string fpsDisplay;

bool isProgramOK;

typedef BOOL (APIENTRY *PFNWGLSWAPINTERVALFARPROC) (int);
PFNWGLSWAPINTERVALFARPROC wglSwapIntervalEXT;


unsigned int getMyRand(){
	static unsigned int m_seed = 2942;

	m_seed = (9683823 * m_seed + 4818229);

	return m_seed % 32767;
}

bool start_log(const char *filename){
	event_log.open(filename);
	if(!event_log.good()){
		std::cout << "Error!: Could not open log file or some shit" << endl;
		return false;
	}
	return true;
}

string getTime(){
	stringstream currentTime;
	if(realTime->tm_hour < 10)
		currentTime << "0";
	currentTime << realTime->tm_hour << ":";
	if(realTime->tm_min < 10)
		currentTime << "0";
	currentTime << realTime->tm_min << ":";
	if(realTime->tm_sec < 10)
		currentTime << "0";
	currentTime << realTime->tm_sec << " ";
	return currentTime.str();
}


void enableVSync(int interval = 1){
	const char *extensions = (const char*)glGetString(GL_EXTENSIONS);

	if(strstr(extensions, "WGL_EXT_swap_control") == 0)
		return;
	else{
		wglSwapIntervalEXT = (PFNWGLSWAPINTERVALFARPROC)wglGetProcAddress("wglSwapIntervalEXT");

		if(wglSwapIntervalEXT)
			wglSwapIntervalEXT(interval);
	}
}


void startDrawing(RenderWindow &handle){
	//handle.pushGLStates();
	//glBindBuffer(GL_ARRAY_BUFFER, 0);
	//glBindVertexArray(0);
	glPushAttrib(GL_COLOR_BUFFER_BIT | GL_CURRENT_BIT   | GL_ENABLE_BIT  |
		GL_TEXTURE_BIT      | GL_TRANSFORM_BIT | GL_VIEWPORT_BIT);
	glDisable(GL_ALPHA_TEST);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_LIGHTING);
	//glDisableVertexAttribArray(0);
}

void stopDrawing(RenderWindow &handle){
	//glEnable(GL_ALPHA_TEST);
	//glEnable(GL_DEPTH_TEST);
	//glEnable(GL_LIGHTING);
	//handle.popGLStates();

	//bindAttribBuffers(vao,vbo,0);
	glPopAttrib();
}

void updateFPS(){
	clock_t fps;
	fps = clock() - oldTime;
	if( ((float)fps/CLOCKS_PER_SEC) >= 1.0){
		fpsCount = frameCount;
		frameCount = 0;
		oldTime = clock();
	}
	
}

int main(int argc, char *argv[]){
	float window_width = 1280,
		window_height = 800;
	RenderWindow window(VideoMode(window_width,window_height),"Shader demo",Style::Fullscreen,ContextSettings(32,0,0,3,3));
	Event event;
	bool isRunning = true;
	isProgramOK = true;

	float startAngleX = -30.0f, startAngleY = 135.0f, startAngleZ = 0.0f;

	float x = 0, y = 0, z = 0;
	float deltaRed = 0.0, deltaGreen = 0.0, deltaBlue = 0.0;
	float angle = 0.0, angleX = 0.0, angleY = 0.0, angleZ = 0.0;
	float deltaAngleX = startAngleX, deltaAngleY = startAngleY, deltaAngleZ = startAngleZ;
	float rightMouseX = 90.0f, rightMouseY = 0.0f;
	Vector3<float> defaultOrigin;

	float m_scale = 1.0;

	//model matrix
	//multiply model by this matrix to get world coordinates
	float m_model[16] =
	{1.0, 0.0, 0.0, 0.0, //first column
	0.0, 1.0, 0.0, 0.0, //second column
	0.0, 0.0, 1.0, 0.0, //third column
	0.0, 0.0, 0.0, 1.0}; //fourth column

	GLfloat cube_vertices[24] = {
		5.0, -5.0, 5.0,
		5.0, 5.0, 5.0,
		-5.0, 5.0, 5.0,
		-5.0, -5.0, 5.0,

		-5.0, -5.0, -5.0,
		-5.0, 5.0, -5.0,
		5.0, 5.0, -5.0,
		5.0, -5.0, -5.0,
	};

	GLubyte cube_indices[36] = {
		0, 1, 2,
		0, 2, 3,
		7, 6, 1,
		7, 1, 0,
		3, 2, 5,
		3, 5, 4,
		4, 5, 6,
		4, 6, 7,
		1, 6, 5,
		1, 5, 2,
		7, 0, 3,
		7, 3, 4
	};


	Text glVersion;
	glVersion.setCharacterSize(20);
	glVersion.setPosition(20, window.getSize().y-50);
	string rendererStr = "Renderer: ",
		versionStr = "OpenGL version: ";
	char rendererText[100], versionText[100];
	
	//FTGLBitmapFont *fpsFont = new FTGLBitmapFont("C:\\Windows\\Fonts\\verdana.ttf");
	//if(fpsFont->Error()){
	//	cout << "Error! Font could not load! Exiting.." << endl;
	//	event_log << "Error! Font could not load! Exiting.." << endl;
	//	return -1;
	//}
	int fontFaceSize = 12;
	//fpsFont->FaceSize(fontFaceSize);

	BitmapFont fpsFont("C:\\Windows\\Fonts\\verdana.ttf", fontFaceSize);

	//fpsFont->Render("HELLO WORLD",-1,FTPoint(10.0,10.0,2.0),FTPoint(),FTGL::RENDER_FRONT | FTGL::RENDER_BACK);

	const GLubyte *renderer = glGetString(GL_RENDERER); //get OpenGL renderer
	const GLubyte *version = glGetString(GL_VERSION); //get OpenGL version


	//NOTE about Perspective:
	//zNear cannot be zero, but values that approach zero 
	//apparently define the maximum render distance
	//i.e. zNear = 0.01 renders targets much farther away 
	//	   than zNear = 0.1
	Camera *cam = Camera::getInstance();
	cam->m_camera_type = cam->c_Flying;

	defaultOrigin.x = -10, defaultOrigin.y = 20, defaultOrigin.z = -10;
	cam->setCamOrigin(defaultOrigin.x, defaultOrigin.y, defaultOrigin.z);
	cam->setUpVec(0,1,0);
	cam->setTarget(128,0,128);
	cam->setOrtho(-0.4, 0.4, -0.3, 0.3, 0.01, 100.0);
	cam->setPerspective(45*DEG2RAD,window_width/window_height,0.1,10000.0); //zNear cannot be zero
	cam->disableZRoll();									

	cam->LookAt();
	

	

	glm::mat4 translation = glm::mat4(1.0);

	int iterations = 3;
	bool firstKeyPress = true;


	int t_n = 256;
	float t_height = 10.0, t_reduction = 1.0, tile_size = 2.0;

	//srand(2);

	//INPUT:	n, height, reduction (H), tile_size
	int terrain_seed = 2314;
	DisplaceTerrain terrain(t_n, t_height, t_reduction, tile_size, cam);
	terrain.setAmbientColor(0.2f);
	terrain.setDiffuseColor(0.8f, 1.0f, 0.9f);
	terrain.setSpecularColor(0.7f);
	terrain.setSeed(terrain_seed);
	//terrain.loadTexture("../../../../../CODING/OpenGL/Textures/MarbleGreen.jpg", GL_TEXTURE_2D);
	terrain.updateCamProperties(cam->getFrustum());
	terrain.construct(false);
	//terrain.constructPerlin2D();
	bool firstTerrainCreated = true;

	//DisplaceTerrain ceiling(t_n, t_height, t_reduction, tile_size, cam->getCamOrigin());
	//glm::mat4 trans = glm::mat4(1.0);
	//trans[3] = glm::vec4(0.0, 100.0, 0.0, 1.0);
	//eng::quat rotationz(glm::vec3(1.0f, 0.0f, 0.0f), 180.0);
	//ceiling.applyModelMatrix(rotationz.getMatrix());
	//ceiling.applyModelMatrix(trans);
	//ceiling.setSeed(terrain_seed);
	//ceiling.setAmbientColor(1.0f);
	//ceiling.setDiffuseColor(0.5f);
	//ceiling.setSpecularColor(0.8f);
	//ceiling.loadTexture("../../../../../CODING/OpenGL/Textures/MarbleGreen.jpg", GL_TEXTURE_2D);
	//ceiling.construct();

	glViewport(0, 0, window.getSize().x,window.getSize().y);

	//Light position
	glm::vec3 lightOrigin(tile_size*t_n/2, 5.0, tile_size*t_n/2);

	float mouseTicks = 0.0;

	float colors[] = {
		1.0, 1.0, 1.0
	};

	if(!start_log("gl.log")){
		//error happened
		return 1;
	} else {
		time(&simpleTime);
		realTime = localtime(&simpleTime);
		event_log << asctime(realTime)
			<< "\n=====================================\n" << endl;
		event_log <<"USS Enterprise Captain's Log: \n" << endl;
	}

	glutInit(&argc,argv);
	GLenum err = glewInit();
	if(GLEW_OK != err){
		//glew screwed up
		event_log << "GLEW error: failed to startup" << endl;
		fprintf(stderr,"Error: %s\n", glewGetErrorString(err));
	}

	MeshList mm;
	string meshDir = MeshDirectory + string("sun_textured.dae");
	mm.loadFile(meshDir.c_str());
	Mesh sun = mm[0];

	//MeshList m2;
	//string meshDir2 = MeshDirectory + string("cube_textured.dae");
	//m2.loadFile(meshDir2.c_str());
	//Mesh cube = m2[0];

	lightOrigin.y = sinf(rightMouseX*DEG2RAD)*(t_n*tile_size/2 + 5.0) + 5.0;
	sun.setAmbientColor(1.0f);
	sun.setDiffuseColor(1.0f);
	sun.setSpecularColor(1.0f);
	sun.setAsLightSource();

	//string texDir = TextureDirectory + string("cube_map_tex.png");
	//cube.setAmbientColor(0.4f);
	//cube.setDiffuseColor(1.0f);
	//cube.setSpecularColor(0.0f);
	//cube.loadTexture(texDir.c_str(), GL_TEXTURE_2D);
	//cube.setModelMatrix(
	//	glm::scale(
	//	glm::translate( glm::mat4(1.0f), glm::vec3(0.0f, 30.0f, 0.0f)),
	//	glm::vec3(10.0f,1.0f,10.0f)
	//	)
	//	);
	//string texDir = TextureDirectory + string("light.png");
	//sun.loadTexture(texDir.c_str(),GL_TEXTURE_2D);

	//Mesh cube; 

	//cube.setVertices(cube_vertices, 24);
	//cube.setVertIndicies(cube_indices, 36);




	//ceiling.loadTexture("../../../../../CODING/OpenGL/Textures/mountain_texture.jpg", GL_TEXTURE_2D);

	////****************************************************************************
	//SHADER INITIALIZATION
	//NOTE: doesnt need to looped over to render stuff
	////****************************************************************************/

	//**********************
	//		Mesh
	//**********************

	//for(int i_m = 0; i_m < mm.size(); i_m++){
	//glm::mat4 stack(1.0);
	//stack[3] = glm::vec4(0.0f, 10.0f*i_m, 0.0f, 1.0f);
	//mm[i_m].applyModelMatrix(stack);
	//mm[i_m].updateModelMatrix();
	//mm[i_m].createAllBuffers();
	//mm[i_m].createVertexArray();
	//mm[i_m].loadTexture("../../../../../CODING/OpenGL/Textures/MarbleGreen0012_1_L.jpg", GL_TEXTURE_2D);
	//}

	//mm[1].loadTexture("../../../../../CODING/OpenGL/Meshes/lilly_model/lilly_eyes_d.dds", GL_TEXTURE_2D);
	terrain.createAllBuffers();
	terrain.createVertexArray();
	//ceiling.createAllBuffers();
	//ceiling.createVertexArray();
	sun.createAllBuffers();
	sun.createVertexArray();
	//cube.createAllBuffers();
	//cube.createVertexArray();


	CustomShader::Shader vertDefault(GL_VERTEX_SHADER), fragDefault(GL_FRAGMENT_SHADER);
	CustomShader::ShaderList shaderProgram;

	//if any shaders fail to load
	if( !(vertDefault.loadFile("../Shaders/render.vert")
		&& fragDefault.loadFile("../Shaders/render.frag")) ){
			std::cout << "Error: Could not load shaders" << endl;
			event_log << "Error: Could not load shaders" << endl;
	}


	//string vertex_shader = vertDefault.getShader(),
	//	fragment_shader = fragDefault.getShader();

	vertDefault.compileShader();
	fragDefault.compileShader();

	event_log << getTime() << "Shaders compiled" << endl;

	shaderProgram.setVert(vertDefault);
	shaderProgram.setFrag(fragDefault);
	shaderProgram.linkShaders();


	if(isProgramOK)
		event_log << getTime() << "Shaders Linked" << endl;

	int numPoints = 0;
	int toggleObj = 0;
	float deltaX = 0, deltaY = 0, deltaZ = 0;
	bool firstClick = true, mouseMovement = false,
		firstClickRight = true, mouseRightMovement = false,
		angleChanged = false, firstPress = true;
	Vector2i mouseOrigin, mouseRightOrigin;
	float rotateVarX = 0.0, rotateVarY = 0.0,
		mouseTravelledY = startAngleX, mouseTravelledX = startAngleY,
		mouseRightTravelledY = 0.0, mouseRightTravelledX = 0.0;
	Vector2<float> center_screen;

	Vector3<float> lookAtPoint;

	center_screen.x = window.getSize().x/2.0;
	center_screen.y = window.getSize().y/2.0;

	float scale = 0.5, speed = 1.0, mouseSpeed = 0.5;
	GLint model_matrix_location = 0,
		view_matrix_location = 0,
		proj_matrix_location = 0,
		cam_origin_location = 0,
		light_origin_location = 0;
	GLuint lutBlockIndex;
	GLfloat *vertPtr;

	if(!isProgramOK)
		exit(-1);

	//get locations of uniforms (used in the shaders)
	model_matrix_location = glGetUniformLocation(shaderProgram.getHandle(),"modelMat");
	view_matrix_location = glGetUniformLocation(shaderProgram.getHandle(),"viewMat");
	proj_matrix_location = glGetUniformLocation(shaderProgram.getHandle(),"projMat");
	cam_origin_location = glGetUniformLocation(shaderProgram.getHandle(),"camOrigin");
	light_origin_location = glGetUniformLocation(shaderProgram.getHandle(),"lightOrigin");

	const char *varNames[] = { "Ka", "Kd", "Ks", "isLightSource", "hasTexture"};

	sun.createUniformBlock(shaderProgram.getHandle(),"MaterialProperties", varNames);
	terrain.createUniformBlock(shaderProgram.getHandle(),"MaterialProperties", varNames);
	//cube.createUniformBlock(shaderProgram.getHandle(),"MaterialProperties", varNames);
	//ceiling.createUniformBlock(shaderProgram.getHandle(),"MaterialProperties", varNames);


	//lutBlockIndex = glGetUniformBlockIndex(shaderProgram.getHandle(), "LUT");
	//glUniformBlockBinding(shaderProgram.getHandle(), lutBlockIndex, uniform_binding_point);

	//glGetActiveUniformBlockiv(shaderProgram.getHandle(), lutBlockIndex, GL_UNIFORM_BLOCK_DATA_SIZE, &blockSize);
	//blockBuffer = (GLubyte*) malloc(blockSize);

	//GLint indices[2];
	//glGetActiveUniformBlockiv(shaderProgram.getHandle(), lutBlockIndex, 
	//	GL_UNIFORM_BLOCK_ACTIVE_UNIFORM_INDICES, indices);

	//GLint offset[2];
	//glGetActiveUniformsiv(shaderProgram.getHandle(), 2, indices, GL_UNIFORM_OFFSET, offset);

	//glGenBuffers(1, &ubo);
	//glBufferData(GL_UNIFORM_BUFFER, sizeof(LUT::case_to_num_polys) + sizeof(LUT::edge_connect_list),



	//**************************************
	//		GL Enables/Disables
	//**************************************

	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_TRUE);
	glDepthFunc(GL_LEQUAL);
	glDepthRange(0.0f, 1.0f);

	glEnable(GL_CULL_FACE);
	glFrontFace(GL_CCW);

	glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );

	glEnable(GL_LINE_SMOOTH);

	enableVSync(1);

	//std::cout << "Displacement points: " << iterations << endl;
	//glBindBuffer(GL_ARRAY_BUFFER, mesh_vbo);
	//glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (GLubyte*)NULL);
	//glBindBuffer(GL_ARRAY_BUFFER, mesh_color_vbo);
	//glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (GLubyte*)NULL);

	std::cout<<"n: "<<t_n<<"\t"
		<<"height: "<<t_height<<"\t"
		<<"reduction: "<<t_reduction<<"\t"
		<<"tile size: "<<tile_size<<endl;

	bool toggle_cull_face = false;

	//****************************************************************************
	//		MAIN LOOP BEGINS
	//****************************************************************************/
	event_log << getTime() << "Begin main rendering loop" << endl;
	//glClearColor(0.5,0.5,0.5,1.0);
	while(isRunning){
		while(window.pollEvent(event)){
			if(event.type == Event::Closed)
				isRunning = false;
			if(event.type == Event::Resized){
				center_screen.x = window.getSize().x/2.0;
				center_screen.y = window.getSize().y/2.0;
			}
			if(Mouse::isButtonPressed(Mouse::Left) && !(event.type == Event::MouseMoved)){
				if(firstClick) {
					mouseOrigin = Mouse::getPosition();
					firstClick = false;
				}
			}
			else if(Mouse::isButtonPressed(Mouse::Right) && !(event.type == Event::MouseMoved)){
				if(firstClickRight) {
					mouseRightOrigin = Mouse::getPosition();
					firstClickRight = false;
				}
			}
			else if(Mouse::isButtonPressed(Mouse::Left) && event.type == Event::MouseMoved){
				//deltaX = 0;
				//deltaY = 0;
				//deltaZ = 0;

				deltaAngleX = fmod(mouseSpeed*(mouseOrigin.y - Mouse::getPosition().y) + mouseTravelledY, 360.0);
				if(deltaAngleX > 90.0)
					deltaAngleX = min(deltaAngleX, 90.0f);
				else if(deltaAngleX <= -90.0)
					deltaAngleX = max(deltaAngleX, -89.9f);
				deltaAngleY = fmod(mouseSpeed*(Mouse::getPosition().x - mouseOrigin.x) + mouseTravelledX, 360.0);
			}
			else if(Mouse::isButtonPressed(Mouse::Right) && event.type == Event::MouseMoved){
				//deltaX = 0;
				//deltaY = 0;
				//deltaZ = 0;
				rightMouseY = fmod((mouseOrigin.y - Mouse::getPosition().y) + mouseRightTravelledY, 360.0);
				rightMouseX = fmod((Mouse::getPosition().x - mouseOrigin.x) + mouseRightTravelledX, 360.0);
			}
			else if(Event::MouseButtonReleased){
				mouseMovement = false;
				mouseRightMovement = false;
				//deltaX = 0;
				//deltaY = 0;
				//deltaZ = 0;


				if(!firstClick){
					mouseTravelledY = deltaAngleX;
					mouseTravelledX = deltaAngleY;
					firstClick = true;
				}
				if(!firstClickRight){
					mouseRightTravelledY = rightMouseY;
					mouseRightTravelledX = rightMouseX;
					firstClickRight = true;
				}
				angleChanged = false;
			}
			if(event.type == Event::MouseWheelMoved){

				if(event.mouseWheel.delta > 0){
					if(mouseTicks >= 0.2)
						mouseTicks -= 0.2;
				}
				else if(event.mouseWheel.delta < 0){
					if(mouseTicks <= 5.0)
						mouseTicks += 0.2;
				}

				cam->setZoom(mouseTicks);
				std::cout<< event.mouseWheel.delta << endl;
			}
			if(event.type == Event::KeyReleased){
				deltaX = 0.0;
				deltaY = 0.0;
				deltaZ = 0.0;
				firstPress = true;
				if(!firstKeyPress){
					speed /= 4;
					firstKeyPress = true;
				}
				firstTerrainCreated = true;

			}
			else if(event.type == sf::Event::KeyPressed){

				if(event.key.code == (Keyboard::Escape)){
					isRunning = false;
				}
				else if(event.key.code == (Keyboard::Tab)){

					if(firstTerrainCreated){
						terrain.updateCamProperties(cam->getFrustum());
						terrain.construct(false);
						//ceiling.construct(false);
						firstTerrainCreated = false;
					}
				}
				else if(event.key.code == Keyboard::LShift){
					if(firstKeyPress){
						speed *= 4;
						firstKeyPress = false;
					}
				}
				else if(event.key.code == (Keyboard::Num1)){
					glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
				}
				else if(event.key.code == (Keyboard::Num2)){
					glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
				}
				else if(event.key.code == (Keyboard::Num0)){
					m_model[0] = 1.0;
					m_model[5] = 1.0;
					m_model[10] = 1.0;
					angleX = 0.0, angleY = 0.0, angleZ = 0.0;
					deltaAngleX = 0.0, deltaAngleY = 0.0, deltaAngleZ = 0.0;
					cam->setCamOrigin(0,50,0);
					cam->setTarget(0,0,0);
					/*cam->lookAtVec.x = 0, cam->lookAtVec.y = 0, cam->lookAtVec.z = -2;*/
					mouseTicks = 0.0;
				}
				else if(event.key.code == (Keyboard::W)){
					deltaZ = speed;
				}
				else if(event.key.code == (Keyboard::S)){
					deltaZ = -speed;
				}
				else if(event.key.code == (Keyboard::A)){
					deltaX = -speed;
				}
				else if(event.key.code == (Keyboard::D)){
					deltaX = speed;
				}
				else if(event.key.code == (Keyboard::R)){
					deltaY = speed;
				}
				else if(event.key.code == (Keyboard::F)){
					deltaY = -speed;

				}
				else if(event.key.code == (Keyboard::Y)){
					if(firstPress){

						firstPress = false;

						cout << "Y Pressed!!" << endl;
					}
				}
				else if(event.key.code == (Keyboard::C)){
					if(firstPress){
						if(toggle_cull_face){
							glEnable(GL_CULL_FACE);
							glFrontFace(GL_CCW);
							firstPress = false;
							toggle_cull_face = !toggle_cull_face;
						}
						else {
							glDisable(GL_CULL_FACE);
							firstPress = false;
							toggle_cull_face = !toggle_cull_face;
						}
					}
				}
				else if(event.key.code == (Keyboard::Right)){
					//deltaX = speed;
					deltaAngleY += PI/2.0;
					//cam->m_target.x += 0.1;
				}
				else if(event.key.code == (Keyboard::Left)){
					//deltaX = -speed;
					deltaAngleY -= PI/2.0;
					//cam->getTarget().x -=0.1;
				}
				else if(event.key.code == (Keyboard::Up)){
					terrain.updateCamProperties(cam->getFrustum());
					if(terrain_seed == 0)
						terrain_seed++;
					terrain_seed *= 2;
					terrain.setSeed(terrain_seed);
					terrain.construct(false); 
					//ceiling.setSeed(terrain_seed);
					//ceiling.construct(false);
				}
				else if(event.key.code == (Keyboard::Down)){
					if(terrain_seed > 1){
						terrain.updateCamProperties(cam->getFrustum());
						terrain_seed /= 2;
						terrain.setSeed(terrain_seed);
						terrain.construct(false);
						//ceiling.setSeed(terrain_seed);
						//ceiling.construct(false);
					}
				}
				else if(event.key.code == (Keyboard::Equal)){
					if(firstTerrainCreated){
						terrain.updateCamProperties(cam->getFrustum());
						t_n*=2;
						tile_size/=2;
						terrain.construct(t_n, t_height, t_reduction, tile_size, false);
						//ceiling.construct(t_n, t_height, t_reduction, tile_size);
						std::cout<<"n: "<<t_n<<"\t"
							<<"height: "<<t_height<<"\t"
							<<"reduction: "<<t_reduction<<"\t"
							<<"tile size: "<<tile_size<<endl;
						firstTerrainCreated = false;
					}
				}
				else if(event.key.code == (Keyboard::Dash)){
					if(firstTerrainCreated){
						terrain.updateCamProperties(cam->getFrustum());
						if(t_n > 1){
							t_n/=2;
							tile_size*=2;

							terrain.construct(t_n, t_height, t_reduction, tile_size, false);
							//ceiling.construct(t_n, t_height, t_reduction, tile_size);
							std::cout<<"n: "<<t_n<<"\t"
								<<"height: "<<t_height<<"\t"
								<<"reduction: "<<t_reduction<<"\t"
								<<"tile size: "<<tile_size<<endl;
						}
						firstTerrainCreated = false;
					}
				}
				else if(event.key.code == (Keyboard::PageUp)){
					if(firstTerrainCreated){
						terrain.updateCamProperties(cam->getFrustum());
						t_height += 1.0;
						terrain.construct(t_n, t_height, t_reduction, tile_size, false);
						//ceiling.construct(t_n, t_height, t_reduction, tile_size);
						std::cout<<"n: "<<t_n<<"\t"
							<<"height: "<<t_height<<"\t"
							<<"reduction: "<<t_reduction<<"\t"
							<<"tile size: "<<tile_size<<endl;
						firstTerrainCreated = false;
					}
				}
				else if(event.key.code == (Keyboard::PageDown)){
					if(firstTerrainCreated){
						terrain.updateCamProperties(cam->getFrustum());
						if(t_height > 1.0){
							t_height -= 1.0;
							terrain.construct(t_n, t_height, t_reduction, tile_size, false);
							//ceiling.construct(t_n, t_height, t_reduction, tile_size);
							std::cout<<"n: "<<t_n<<"\t"
								<<"height: "<<t_height<<"\t"
								<<"reduction: "<<t_reduction<<"\t"
								<<"tile size: "<<tile_size<<endl;
						}
						firstTerrainCreated = false;
					}
				}
				else if(event.key.code == (Keyboard::I)){
					if(firstPress) {
						//terrain.IncreaseDetail();
						firstPress = false;
					}
				}
				else if(event.key.code == (Keyboard::K)){
					if(firstPress) {
						//terrain.DecreaseDetail();
						firstPress = false;
					}
				}
				else if(event.key.code == (Keyboard::J)){
					deltaAngleY -= 0.1;
				}
				else if(event.key.code == (Keyboard::L)){
					deltaAngleY += 0.1;
				}
				else if(event.key.code == (Keyboard::O)){
					deltaAngleZ -= 0.1;
				}
				else if(event.key.code == (Keyboard::P)){
					deltaAngleZ += 0.1;
				}
			} //keypressed

			else {

			} 
		}

		cam->move(deltaX, deltaY, deltaZ);

		//std::cout << "X: (";
		//std::cout 
		//	<< setprecision(1) << cam->rightVec.x << "," 
		//	<< setprecision(1) << cam->rightVec.y << ","
		//	<< setprecision(1) << cam->rightVec.z << ") \tY: ("
		//	<< setprecision(1) << cam->upVec.x << "," 
		//	<< setprecision(1) << cam->upVec.y << ","
		//	<< setprecision(1) << cam->upVec.z << ") \tZ: ("
		//	<< setprecision(1) << cam->lookAtVec.x << "," 
		//	<< setprecision(1) << cam->lookAtVec.y << ","
		//	<< setprecision(1) << cam->lookAtVec.z << ")" << endl;

		//event_log << "X: (";
		//event_log 
		//	<< setprecision(1) << cam->rightVec.x << "," 
		//	<< setprecision(1) << cam->rightVec.y << ","
		//	<< setprecision(1) << cam->rightVec.z << ") \tY: ("
		//	<< setprecision(1) << cam->upVec.x << "," 
		//	<< setprecision(1) << cam->upVec.y << ","
		//	<< setprecision(1) << cam->upVec.z << ") \tZ: ("
		//	<< setprecision(1) << cam->lookAtVec.x << "," 
		//	<< setprecision(1) << cam->lookAtVec.y << ","
		//	<< setprecision(1) << cam->lookAtVec.z << ")" << endl;

		cam->updateCamera(deltaAngleX, deltaAngleY);

		//rotate light around the place
		lightOrigin.y = sinf(rightMouseX*DEG2RAD)*(t_n*tile_size/2);
		lightOrigin.z = -cosf(rightMouseX*DEG2RAD)*(t_n*tile_size/2) + tile_size*t_n/2;


		//1.0f/(0.05*t_n*tile_size);
		sun.setModelMatrix(
			glm::scale(
				glm::translate(glm::mat4(1.0), lightOrigin),
				glm::vec3(t_n*tile_size*0.009375f)
			)
		);

		//cout << lightOrigin.x << " " << lightOrigin.y << " " << lightOrigin.z << endl;

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		//model_matrix_location = glGetUniformLocation(shaderProgram.getHandle(),"scale");
		//glUniform1f(model_matrix_location,scale;

		glUseProgram(shaderProgram.getHandle());

		//get uniform vars
		glm::vec3 camPos(cam->getCamOrigin());
		glUniform3fv(cam_origin_location, 1, glm::value_ptr(camPos));
		glUniform3fv(light_origin_location, 1, glm::value_ptr(lightOrigin));


		//get uniform matricies
		glUniformMatrix4fv(view_matrix_location, 1, GL_FALSE, cam->getViewMatrix());
		glUniformMatrix4fv(proj_matrix_location, 1, GL_FALSE, cam->getProjMatrix());

		//Binding VAO again restores all buffer bindings
		//and attribute settings that were set up previously
		//for(int j = 0; j < mm.size(); j++){
		//	glUniformMatrix4fv(model_matrix_location, 1, GL_FALSE, mm[j].getModelMatrix());
		//	//glUniform3fv(diffuse_reflectance_location, 1, terrain.getDiffuseReflectance());
		//	mm[j].updateVertexArray();
		//	glDrawArrays(GL_TRIANGLES, 0, mm[j].getNumVertices());
		//}

		//glm::mat4 modelViewMat = cam->getViewMat4() * terrain.getModelMat4();
		//modelViewMat = glm::inverse(modelViewMat);
		//modelViewMat = glm::transpose(modelViewMat);

		//Update mesh LOD and vertex array
		glUniformMatrix4fv(model_matrix_location, 1, GL_FALSE, terrain.getModelMatrix());
		//terrain.setDiffuseColor(0.8f, 1.0f, 0.9f);
		terrain.updateCamProperties(cam->getFrustum());
		terrain.updateVertexArray(false);
		glDrawArrays(GL_TRIANGLES, 0, terrain.getNumTriangles()*3);
		//terrain.setAsLightSource();
		//terrain.setDiffuseColor(0.4f);
		//terrain.updateVertexArray(true);
		//glDrawArrays(GL_LINES, 0, terrain.getNumTriangles()*8);
		//terrain.unsetLightSource();

		//glUniformMatrix4fv(model_matrix_location, 1, GL_FALSE, ceiling.getModelMatrix());
		//ceiling.updateVertexArray();
		//glDrawArrays(GL_TRIANGLES, 0, ceiling.getNumVertices());

		glUniformMatrix4fv(model_matrix_location, 1, GL_FALSE, sun.getModelMatrix());
		sun.updateVertexArray();
		glDrawArrays(GL_TRIANGLES, 0, sun.getNumVertices());

		//glUniformMatrix4fv(model_matrix_location, 1, GL_FALSE, cube.getModelMatrix());
		//cube.updateVertexArray();
		//glDrawArrays(GL_TRIANGLES, 0, cube.getNumVertices());


		updateFPS();
		fpsFont.renderText(50, 10.0, window.getSize().y-fontFaceSize, "FPS: %d", fpsCount);
		fpsFont.renderText(50, 10.0, window.getSize().y-fontFaceSize*2, "n = %d", t_n);
		fpsFont.renderText(50, 10.0, window.getSize().y-fontFaceSize*3, "height = %.0f", t_height);
		fpsFont.renderText(100, 10.0, window.getSize().y-fontFaceSize*4, "Cam X: %.2f  Cam Y: %.2f  Cam Z: %.2f", 
			cam->getCamOrigin().x, cam->getCamOrigin().y, cam->getCamOrigin().z);
		fpsFont.renderText(100, 10.0, window.getSize().y-fontFaceSize*5, "Lookat X: %.2f  Lookat Y: %.2f  Lookat Z: %.2f", 
			cam->getTarget().x, cam->getTarget().y, cam->getTarget().z);
		fpsFont.renderText(50, 10.0, window.getSize().y-fontFaceSize*6, "Triangles rendered: %d", terrain.getNumTriangles());
		fpsFont.renderText(50, 10.0, window.getSize().y-fontFaceSize*7, "Min. Distance: %.2f", terrain.getCenterDistance());

		window.display();
		frameCount++;
	}
	///****************************************************************************
	//		MAIN LOOP ENDS
	//****************************************************************************/
	event_log << getTime() << "Main Loop ended" <<endl;

	event_log << "End of log.\n"
		<< "=====================================\n"<< endl;
	event_log.close();

	//delete fpsFont;
	return 0;
}
