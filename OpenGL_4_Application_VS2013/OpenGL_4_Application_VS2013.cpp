
#define GLEW_STATIC

#include <iostream>
#include "glm/glm.hpp"//core glm functionality
#include "glm/gtc/matrix_transform.hpp"//glm extension for generating common transformation matrices
#include "glm/gtc/matrix_inverse.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "GLEW/glew.h"
#include "GLFW/glfw3.h"
#include <string>
#include "Shader.hpp"
#include "Camera.hpp"
#include "SkyBox.hpp"
#define TINYOBJLOADER_IMPLEMENTATION

#include "Model3D.hpp"
#include "Mesh.hpp"

void RenderCube();
void RenderQuad();
const GLuint SHADOW_WIDTH = 1024, SHADOW_HEIGHT = 1024;
const GLuint SCR_WIDTH = 1920, SCR_HEIGHT = 1200;

int glWindowWidth = 1920;
int glWindowHeight = 1200;
int retina_width, retina_height;
GLFWwindow* glWindow = NULL;
GLfloat lastX = glWindowWidth / 2, lastY = glWindowHeight / 2;
bool firstMouse = true;


glm::mat4 model;
GLuint modelLoc;



glm::mat4 view;
GLuint viewLoc;
glm::mat4 projection;
GLuint projectionLoc;
glm::mat3 normalMatrix;
GLuint normalMatrixLoc;

glm::vec3 lightDir;
GLuint lightDirLoc;
glm::vec3 lightColor;
GLuint lightColorLoc;

gps::Camera myCamera(glm::vec3(-4.0f, 0.0f, 6.0f));
float cameraSpeed = 0.01f;

bool pressedKeys[1024];
float angle = 0.0f;
float moonAngle = 0.0f;
float delta = 0;
float animAngle = 0.0f;
bool addSky = false;


gps::Model3D myModel;
gps::Model3D wallModel;
gps::Model3D lufModel;
gps::Model3D sunModel;
gps::Model3D castleModel;
gps::Model3D moonModel;
gps::Model3D terModel;
gps::Model3D treeModel;
gps::Model3D grModel;





gps::Shader myCustomShader;
gps::Shader myShadowShader;
gps::Shader myLightShader;
gps::Shader debugDepthQuad;


gps::SkyBox mySkyBox;
gps::Shader skyboxShader;

GLuint skyVAO;
GLuint shadowMapFBO;
glm::vec3 lightDirection = glm::vec3(-2.0f, -0.5f, -15.0f);
glm::vec3 lightPos(10.0f);


GLuint ReadTextureFromFile(const char* file_name) {
	int x, y, n;
	int force_channels = 4;
	unsigned char* image_data = stbi_load(file_name, &x, &y, &n, force_channels);
	if (!image_data) {
		fprintf(stderr, "ERROR: could not load %s\n", file_name);
		return false;
	}
	// NPOT check
	if ((x & (x - 1)) != 0 || (y & (y - 1)) != 0) {
		fprintf(
			stderr, "WARNING: texture %s is not power-of-2 dimensions\n", file_name
			);
	}

	int width_in_bytes = x * 4;
	unsigned char *top = NULL;
	unsigned char *bottom = NULL;
	unsigned char temp = 0;
	int half_height = y / 2;

	for (int row = 0; row < half_height; row++) {
		top = image_data + row * width_in_bytes;
		bottom = image_data + (y - row - 1) * width_in_bytes;
		for (int col = 0; col < width_in_bytes; col++) {
			temp = *top;
			*top = *bottom;
			*bottom = temp;
			top++;
			bottom++;
		}
	}

	GLuint textureID;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_2D, textureID);
	glTexImage2D(
		GL_TEXTURE_2D,
		0,
		GL_SRGB, //GL_SRGB,//GL_RGBA,
		x,
		y,
		0,
		GL_RGBA,
		GL_UNSIGNED_BYTE,
		image_data
		);
	glGenerateMipmap(GL_TEXTURE_2D);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glBindTexture(GL_TEXTURE_2D, 0);

	return textureID;
}


GLenum glCheckError_(const char *file, int line)
{
	GLenum errorCode;
	while ((errorCode = glGetError()) != GL_NO_ERROR)
	{
		std::string error;
		switch (errorCode)
		{
		case GL_INVALID_ENUM:                  error = "INVALID_ENUM"; break;
		case GL_INVALID_VALUE:                 error = "INVALID_VALUE"; break;
		case GL_INVALID_OPERATION:             error = "INVALID_OPERATION"; break;
		case GL_STACK_OVERFLOW:                error = "STACK_OVERFLOW"; break;
		case GL_STACK_UNDERFLOW:               error = "STACK_UNDERFLOW"; break;
		case GL_OUT_OF_MEMORY:                 error = "OUT_OF_MEMORY"; break;
		case GL_INVALID_FRAMEBUFFER_OPERATION: error = "INVALID_FRAMEBUFFER_OPERATION"; break;
		}
		std::cout << error << " | " << file << " (" << line << ")" << std::endl;
	}
	return errorCode;
}
#define glCheckError() glCheckError_(__FILE__, __LINE__)

void windowResizeCallback(GLFWwindow* window, int width, int height)
{
	fprintf(stdout, "window resized to width: %d , and height: %d\n", width, height);
	//TODO
	//for RETINA display
	glfwGetFramebufferSize(glWindow, &retina_width, &retina_height);

	//set projection matrix
	glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)retina_width / (float)retina_height, 0.1f, 1000.0f);
	//send matrix data to shader
	GLint projLoc = glGetUniformLocation(myCustomShader.shaderProgram, "projection");
	glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

	//set Viewport transform
	glViewport(0, 0, retina_width, retina_height);
}

void keyboardCallback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);

	if (key >= 0 && key < 1024)
	{
		if (action == GLFW_PRESS)
			pressedKeys[key] = true;
		else if (action == GLFW_RELEASE)
			pressedKeys[key] = false;
	}
}

void mouseCallback(GLFWwindow* window, double xpos, double ypos)
{
	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	GLfloat xoffset = xpos - lastX;
	GLfloat yoffset = lastY - ypos;

	lastX = xpos;
	lastY = ypos;

	myCamera.processMouseMovement(xoffset, yoffset, GL_FALSE);
}
float movementSpeed = 0.7; // units per second
void updateAngle(double elapsedSeconds) {
	angle = angle - movementSpeed * elapsedSeconds;
}
double lastTimeStamp = glfwGetTime();
bool startAnim = false;
void processMovement()
{

	if (pressedKeys[GLFW_KEY_Q]) {
		angle += 0.1f;
		if (angle > 360.0f)
			angle -= 360.0f;
	}

	if (pressedKeys[GLFW_KEY_E]) {
		angle -= 0.1f;
		if (angle < 0.0f)
			angle += 360.0f;
	}
	if (pressedKeys[GLFW_KEY_SPACE]) {
		startAnim = true;
	}

	if (pressedKeys[GLFW_KEY_W]) {
		myCamera.processKeyboard(gps::FORWARD, cameraSpeed);
	}

	if (pressedKeys[GLFW_KEY_S]) {
		myCamera.processKeyboard(gps::BACKWARD, cameraSpeed);
	}

	if (pressedKeys[GLFW_KEY_A]) {
		myCamera.processKeyboard(gps::LEFT, cameraSpeed);
	}

	if (pressedKeys[GLFW_KEY_D]) {
		myCamera.processKeyboard(gps::RIGHT, cameraSpeed);
	}

	if (pressedKeys[GLFW_KEY_UP]) {
		myCamera.processKeyboard(gps::FORWARD, cameraSpeed);
	
	}

	if (pressedKeys[GLFW_KEY_DOWN]) {
		myCamera.processKeyboard(gps::BACKWARD, cameraSpeed);
		
	}

	if (pressedKeys[GLFW_KEY_LEFT]) {
		myCamera.processKeyboard(gps::LEFT, cameraSpeed);
		
	}

	if (pressedKeys[GLFW_KEY_RIGHT]) {
		myCamera.processKeyboard(gps::RIGHT, cameraSpeed);
		
	}

	if (pressedKeys[GLFW_KEY_P]) {
		glUniform1i(glGetUniformLocation(myCustomShader.shaderProgram, "useDirLight"), 0);
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	}
	if (pressedKeys[GLFW_KEY_O]) {
		glUniform1i(glGetUniformLocation(myCustomShader.shaderProgram, "usePointLight"), 0);
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	}
	if (pressedKeys[GLFW_KEY_I]) {
		glUniform1i(glGetUniformLocation(myCustomShader.shaderProgram, "useSpotLight"), 0);
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	}
	if (pressedKeys[GLFW_KEY_TAB]) {
		glUniform1i(glGetUniformLocation(myCustomShader.shaderProgram, "useFog"), 1);
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	}
	if (pressedKeys[GLFW_KEY_CAPS_LOCK]) {
		glUniform1i(glGetUniformLocation(myCustomShader.shaderProgram, "useFog"), 0);
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	}
	if (pressedKeys[GLFW_KEY_L]) {
		moonAngle += 0.2f;
	}
	if (pressedKeys[GLFW_KEY_K]) {
		moonAngle -= 0.2f;
	}
	if (pressedKeys[GLFW_KEY_X]) {
		delta += 0.2f;
	}
	if (pressedKeys[GLFW_KEY_Z]) {
		delta -= 0.2f;
	}
	if (pressedKeys[GLFW_KEY_F]) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	}
	if (pressedKeys[GLFW_KEY_G]) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}
	if (pressedKeys[GLFW_KEY_T]) {
		addSky = true;
	}
	if (pressedKeys[GLFW_KEY_Y]) {
		addSky = false;
	}

}

bool initOpenGLWindow()
{
	if (!glfwInit()) {
		fprintf(stderr, "ERROR: could not start GLFW3\n");
		return false;
	}

	//for Mac OS X
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	glWindow = glfwCreateWindow(glWindowWidth, glWindowHeight, "OpenGL Shader Example", NULL, NULL);
	if (!glWindow) {
		fprintf(stderr, "ERROR: could not open window with GLFW3\n");
		glfwTerminate();
		return false;
	}

	glfwSetWindowSizeCallback(glWindow, windowResizeCallback);
	glfwMakeContextCurrent(glWindow);

	glfwWindowHint(GLFW_SAMPLES, 4);

	// start GLEW extension handler
	glewExperimental = GL_TRUE;
	glewInit();

	// get version info
	const GLubyte* renderer = glGetString(GL_RENDERER); // get renderer string
	const GLubyte* version = glGetString(GL_VERSION); // version as a string
	printf("Renderer: %s\n", renderer);
	printf("OpenGL version supported %s\n", version);

	//for RETINA display
	glfwGetFramebufferSize(glWindow, &retina_width, &retina_height);

	glfwSetKeyCallback(glWindow, keyboardCallback);
	glfwSetCursorPosCallback(glWindow, mouseCallback);
	//glfwSetInputMode(glWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	return true;
}

void initOpenGLState()
{
	glClearColor(0.3, 0.3, 0.3, 1.0);
	glViewport(0, 0, retina_width, retina_height);

	glEnable(GL_DEPTH_TEST); // enable depth-testing
	glDepthFunc(GL_LESS); // depth-testing interprets a smaller value as "closer"
	glEnable(GL_CULL_FACE); // cull face
	glCullFace(GL_BACK); // cull back face
	glFrontFace(GL_CCW); // GL_CCW for counter clock-wise
}

void initModels()
{

	//lufModel = gps::Model3D("objects/Luffy/luffychild.obj", "objects/Luffy/");
	lufModel = gps::Model3D("objects/Bird/Steampunk Airship/Airship.obj", "objects/Bird/Steampunk Airship/");
	wallModel = gps::Model3D("objects/house/habitat.obj", "objects/house/");
	castleModel = gps::Model3D("objects/Castle/castle.obj", "objects/Castle/");
	moonModel = gps::Model3D("objects/Moon/moon.obj", "objects/Moon/");
	terModel = gps::Model3D("objects/cube/sky.obj", "objects/cube/");
	treeModel = gps::Model3D("objects/Tree/default_OBJ/tree.obj", "objects/Tree/default_OBJ/");
	grModel = gps::Model3D("objects/Tree/default_OBJ/gravestone01.obj", "objects/Tree/default_OBJ/");
}

void initShaders()
{
	//myCustomShader.loadShader("shaders/shaderVert.vert", "shaders/shaderFrag.frag");
	myCustomShader.loadShader("shaders/advancedLighting.vert", "shaders/advancedLighting.frag");
	myShadowShader.loadShader("shaders/shadowShader.vert", "shaders/shadowShader.frag");
	debugDepthQuad.loadShader("shaders/sceneShader.vert", "shaders/sceneShader.frag");
	myLightShader.loadShader("shaders/lampShader.vert", "shaders/lampShader.frag");
	skyboxShader.loadShader ("shaders/skyboxShader.vert","shaders/skyboxShader.frag");
	skyboxShader.useShaderProgram();
}
//the uniforms for light
void initUniformsLight(gps::Shader shader)
{


	glm::mat4 projection = glm::perspective(myCamera.zoom, (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 1000.0f);
	glm::mat4 view = myCamera.getViewMatrix();
	glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
	glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
	// Set light uniforms
	//glUniform3fv(glGetUniformLocation(shader.shaderProgram, "lightPos"), 1, &lightPos[0]);
	glUniform3fv(glGetUniformLocation(shader.shaderProgram, "dirLight.position"), 1, &lightPos[0]);
	glUniform3fv(glGetUniformLocation(shader.shaderProgram, "viewPos"), 1, &myCamera.Position[0]);
	//init skyBox shader
	glUniformMatrix4fv(glGetUniformLocation(skyboxShader.shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
	glUniformMatrix4fv(glGetUniformLocation(skyboxShader.shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));



	glm::vec3 lightcolor = glm::vec3(1.0f, 1.0f, 1.0f);
	glm::vec3 toycolor = glm::vec3(1.0f, 0.5f, 0.31f);
	glm::vec3 result = lightcolor*toycolor;

	glm::vec3 viewPos = myCamera.Position;


	GLuint viewPosLoc = glGetUniformLocation(shader.shaderProgram, "viewPos");
	glUniform3fv(viewPosLoc, 1, value_ptr(viewPos));

	GLuint matShineLoc = glGetUniformLocation(shader.shaderProgram, "material.shininess");
	glm::vec3 matSpec = glm::vec3(0.5f, 0.5f, 0.5f);
	float matShine = 32.0f;
	glUniform1f(matShineLoc, matShine);

	glm::vec3 lightAmbient = glm::vec3(0.1f, 0.1f, 0.1f);
	glm::vec3 lightSpec = glm::vec3(1.0f, 1.0f, 1.0f);
	glm::vec3 lightDiff = glm::vec3(0.8f, 0.8f, 0.8f);

	//glm::vec3 lightPos = myCamera.Position;
	glm::vec3 lightSpotDir = myCamera.Front;
	float cutOff = glm::cos(glm::radians(12.5));
	float outerCutOff = glm::cos(glm::radians(17.5));
	//define  the uniforms for the fragShader

	//the point lights
	glUniform1f(glGetUniformLocation(shader.shaderProgram, "pointLights[0].constant"), 1.0f);
	glUniform1f(glGetUniformLocation(shader.shaderProgram, "pointLights[1].constant"), 1.0f);
	glUniform1f(glGetUniformLocation(shader.shaderProgram, "pointLights[2].constant"), 1.0f);
	glUniform1f(glGetUniformLocation(shader.shaderProgram, "pointLights[3].constant"), 1.0f);

	glUniform1f(glGetUniformLocation(shader.shaderProgram, "pointLights[0].linear"), 0.09f);
	glUniform1f(glGetUniformLocation(shader.shaderProgram, "pointLights[1].linear"), 0.09f);
	glUniform1f(glGetUniformLocation(shader.shaderProgram, "pointLights[2].linear"), 0.09f);
	glUniform1f(glGetUniformLocation(shader.shaderProgram, "pointLights[3].linear"), 0.09f);

	glUniform1f(glGetUniformLocation(shader.shaderProgram, "pointLights[0].quadratic"), 0.032f);
	glUniform1f(glGetUniformLocation(shader.shaderProgram, "pointLights[1].quadratic"), 0.032f);
	glUniform1f(glGetUniformLocation(shader.shaderProgram, "pointLights[2].quadratic"), 0.032f);
	glUniform1f(glGetUniformLocation(shader.shaderProgram, "pointLights[3].quadratic"), 0.032f);

	//vec3 for components of light
	glUniform3fv((glGetUniformLocation(shader.shaderProgram, "pointLights[0].ambient")), 1, glm::value_ptr(lightAmbient));
	glUniform3fv((glGetUniformLocation(shader.shaderProgram, "pointLights[1].ambient")), 1, glm::value_ptr(lightAmbient));
	glUniform3fv((glGetUniformLocation(shader.shaderProgram, "pointLights[2].ambient")), 1, glm::value_ptr(lightAmbient));
	glUniform3fv((glGetUniformLocation(shader.shaderProgram, "pointLights[3].ambient")), 1, glm::value_ptr(lightAmbient));

	glUniform3fv((glGetUniformLocation(shader.shaderProgram, "pointLights[0].diffuse")), 1, glm::value_ptr(lightDiff));
	glUniform3fv((glGetUniformLocation(shader.shaderProgram, "pointLights[1].diffuse")), 1, glm::value_ptr(lightDiff));
	glUniform3fv((glGetUniformLocation(shader.shaderProgram, "pointLights[2].diffuse")), 1, glm::value_ptr(lightDiff));
	glUniform3fv((glGetUniformLocation(shader.shaderProgram, "pointLights[3].diffuse")), 1, glm::value_ptr(lightDiff));

	glUniform3fv((glGetUniformLocation(shader.shaderProgram, "pointLights[0].specular")), 1, glm::value_ptr(lightSpec));
	glUniform3fv((glGetUniformLocation(shader.shaderProgram, "pointLights[1].specular")), 1, glm::value_ptr(lightSpec));
	glUniform3fv((glGetUniformLocation(shader.shaderProgram, "pointLights[2].specular")), 1, glm::value_ptr(lightSpec));
	glUniform3fv((glGetUniformLocation(shader.shaderProgram, "pointLights[3].specular")), 1, glm::value_ptr(lightSpec));

	//vec3 position for lights
	glm::vec3 pointLightPositions[] = {
		glm::vec3(0.7f + delta, 0.2f, 2.0f),
		glm::vec3(2.3f, -3.3f - delta, -4.0f),
		glm::vec3(-4.0f + delta, 2.0f, -12.0f - delta),
		glm::vec3(0.0f - delta, 0.0f + delta, -3.0f)
	};
	glUniform3fv((glGetUniformLocation(shader.shaderProgram, "pointLights[0].position")), 1, glm::value_ptr(pointLightPositions[0]));
	glUniform3fv((glGetUniformLocation(shader.shaderProgram, "pointLights[1].position")), 1, glm::value_ptr(pointLightPositions[1]));
	glUniform3fv((glGetUniformLocation(shader.shaderProgram, "pointLights[2].position")), 1, glm::value_ptr(pointLightPositions[2]));
	glUniform3fv((glGetUniformLocation(shader.shaderProgram, "pointLights[3].position")), 1, glm::value_ptr(pointLightPositions[3]));

	//the directional light
	glUniform3f((glGetUniformLocation(shader.shaderProgram, "dirLight.ambient")), 0.9f, 0.9f, 0.9f);
	glUniform3f((glGetUniformLocation(shader.shaderProgram, "dirLight.diffuse")), 0.4f, 0.4f, 0.4);
	glUniform3f((glGetUniformLocation(shader.shaderProgram, "dirLight.specular")), 0.5f, 0.5f, -0.5f);
	glUniform3f((glGetUniformLocation(shader.shaderProgram, "dirLight.position")), lightPos.x + moonAngle, lightPos.y + moonAngle, lightPos.z + moonAngle);
	//glUniform3fv(glGetUniformLocation(shader.shaderProgram, "dirLight.position"), 1, &lightPos[0]);

	//add spot light

	glUniform1f(glGetUniformLocation(shader.shaderProgram, "spotLight.constant"), 1.0f);
	glUniform1f(glGetUniformLocation(shader.shaderProgram, "spotLight.linear"), 0.09);
	glUniform1f(glGetUniformLocation(shader.shaderProgram, "spotLight.quadratic"), 0.032);
	glUniform3fv((glGetUniformLocation(shader.shaderProgram, "spotLight.direction")), 1, glm::value_ptr(lightSpotDir));
	glUniform3fv((glGetUniformLocation(shader.shaderProgram, "spotLight.position")), 1, glm::value_ptr(viewPos));
	glm::vec3 lightAmbient1 = glm::vec3(0.9f, 0.9f, 0.9f);
	glUniform3fv((glGetUniformLocation(shader.shaderProgram, "spotLight.ambient")), 1, glm::value_ptr(lightAmbient1));
	glUniform3fv((glGetUniformLocation(shader.shaderProgram, "spotLight.diffuse")), 1, glm::value_ptr(lightDiff));
	glUniform3fv((glGetUniformLocation(shader.shaderProgram, "spotLight.specular")), 1, glm::value_ptr(lightSpec));
	glUniform1f(glGetUniformLocation(shader.shaderProgram, "spotLight.cutOff"), cutOff);
	glUniform1f(glGetUniformLocation(shader.shaderProgram, "spotLight.outerCutOff"), outerCutOff);
	//glUniform3fv(viewPosLoc, 1, value_ptr(viewPos));

	glUniform1i(glGetUniformLocation(shader.shaderProgram, "useDirLight"), 1);
	glUniform1i(glGetUniformLocation(shader.shaderProgram, "usePointLight"), 1);
	glUniform1i(glGetUniformLocation(shader.shaderProgram, "useSpotLight"), 1);
	model = glm::mat4(1.0f);
	modelLoc = glGetUniformLocation(shader.shaderProgram, "model");
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

}
void initSkyBox()
{
	std::vector<const GLchar*> faces;
	faces.push_back("textures/skybox/right.tga");
	faces.push_back("textures/skybox/left.tga");
	faces.push_back("textures/skybox/top.tga");
	faces.push_back("textures/skybox/bottom.tga");
	faces.push_back("textures/skybox/back.tga");
	faces.push_back("textures/skybox/front.tga");
	mySkyBox.Load(faces);

	/*std::vector<const GLchar*> faces;
	faces.push_back("textures/skybox/ss_rt.tga");//right
	faces.push_back("textures/skybox/ss_lf.tga");//left
	faces.push_back("textures/skybox/ss_up.tga");//up
	faces.push_back("textures/skybox/ss_dn.tga");//down
	faces.push_back("textures/skybox/ss_bk.tga");//back
	faces.push_back("textures/skybox/ss_ft.tga");//front
	mySkyBox.Load(faces);*/
}

void renderBackground(gps::Shader shader)
{
	
}




void renderScene(gps::Shader shader)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//processMovement();

	//initialize the view matrix
	view = myCamera.getViewMatrix();
	//send view matrix data to shader	
	glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));

	//initialize the model matrix
	model = glm::mat4(1.0f);
	//create model matrix
	model = glm::translate(model, glm::vec3(-8.0f, 0.0f, 0.0f));
	model = glm::rotate(model, glm::radians(angle), glm::vec3(0, 1, 0));
	if (startAnim)
	{
		double currentTimeStamp = glfwGetTime();
		updateAngle(currentTimeStamp-lastTimeStamp);
		lastTimeStamp = currentTimeStamp;
		model = glm::rotate(model, glm::radians(angle), glm::vec3(0, 1, 0));
	}
	//send model matrix data to shader	
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	processMovement();



	model = glm::translate(model, glm::vec3(10.0f, 30.0f, 0.0f));
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	glActiveTexture(GL_TEXTURE0);
	glUniform1i(glGetUniformLocation(myCustomShader.shaderProgram, "diffuseTexture"), 0);
	glUniform1i(glGetUniformLocation(shader.shaderProgram, "material.diffuse"), 0);

	lufModel.Draw(shader);

	

	model = glm::mat4();
	model = glm::translate(model, glm::vec3(-100.0f, -5.3f, -100.1f));
	model = glm::scale(model, glm::vec3(0.7));
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	glActiveTexture(GL_TEXTURE0);
	glUniform1i(glGetUniformLocation(myCustomShader.shaderProgram, "diffuseTexture"), 0);
	glUniform1i(glGetUniformLocation(shader.shaderProgram, "material.diffuse"), 0);
	
	castleModel.Draw(shader);

	model = glm::translate(model, glm::vec3(200.0f, -17.5f, 60.0f));
	model = glm::scale(model, glm::vec3(0.2));
	model = glm::rotate(model, glm::radians(200.0f), glm::vec3(0, 1, 0));
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	glActiveTexture(GL_TEXTURE0);
	glUniform1i(glGetUniformLocation(myCustomShader.shaderProgram, "diffuseTexture"), 0);
	glUniform1i(glGetUniformLocation(shader.shaderProgram, "material.diffuse"), 0);
	wallModel.Draw(shader);


	model = glm::mat4();
	model = glm::rotate(model, glm::radians(moonAngle), glm::vec3(1, 0, 0));
	model = glm::translate(model, lightPos);
	model = glm::scale(model, glm::vec3(0.02));
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	glActiveTexture(GL_TEXTURE0);
	glUniform1i(glGetUniformLocation(myCustomShader.shaderProgram, "diffuseTexture"), 0);
	glUniform1i(glGetUniformLocation(shader.shaderProgram, "material.diffuse"), 0);
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	moonModel.Draw(shader);

	model = glm::mat4();
	model = glm::translate(model, glm::vec3(-100.0f, 131.0f, 0.1f));
	model = glm::scale(model, glm::vec3(200.0));
	model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0, 0, 1));
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	glActiveTexture(GL_TEXTURE0);
	glUniform1i(glGetUniformLocation(myCustomShader.shaderProgram, "diffuseTexture"), 0);
	glUniform1i(glGetUniformLocation(shader.shaderProgram, "material.diffuse"), 0);
	terModel.Draw(shader);

	model = glm::mat4();
	model = glm::translate(model, glm::vec3(-120.0f,100.0f, 0.1f));
	model = glm::scale(model, glm::vec3(200.0));
	model = glm::rotate(model, glm::radians(180.0f), glm::vec3(0, 0, 1));
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	glActiveTexture(GL_TEXTURE0);
	glUniform1i(glGetUniformLocation(myCustomShader.shaderProgram, "diffuseTexture"), 0);
	glUniform1i(glGetUniformLocation(shader.shaderProgram, "material.diffuse"), 0);
	terModel.Draw(shader);
	
	model = glm::mat4();
	model = glm::translate(model, glm::vec3(100.0f, 100.0f, 0.1f));
	model = glm::scale(model, glm::vec3(200.0));
	model = glm::rotate(model, glm::radians(180.0f), glm::vec3(1, 0, 0));
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	glActiveTexture(GL_TEXTURE0);
	glUniform1i(glGetUniformLocation(myCustomShader.shaderProgram, "diffuseTexture"), 0);
	glUniform1i(glGetUniformLocation(shader.shaderProgram, "material.diffuse"), 0);
	terModel.Draw(shader);

	model = glm::mat4();
	model = glm::translate(model, glm::vec3(0.0f, 100.0f, 100.1f));
	model = glm::scale(model, glm::vec3(200.0));
	model = glm::rotate(model, glm::radians(180.0f), glm::vec3(1, 0, 0));
	model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0, 1, 0));
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	glActiveTexture(GL_TEXTURE0);
	glUniform1i(glGetUniformLocation(myCustomShader.shaderProgram, "diffuseTexture"), 0);
	glUniform1i(glGetUniformLocation(shader.shaderProgram, "material.diffuse"), 0);
	terModel.Draw(shader);

	model = glm::mat4();
	model = glm::translate(model, glm::vec3(0.0f, 100.0f, -120.1f));
	model = glm::scale(model, glm::vec3(200.0));
	model = glm::rotate(model, glm::radians(180.0f), glm::vec3(1, 0, 0));
	model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(0, 1, 0));
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	glActiveTexture(GL_TEXTURE0);
	glUniform1i(glGetUniformLocation(myCustomShader.shaderProgram, "diffuseTexture"), 0);
	glUniform1i(glGetUniformLocation(shader.shaderProgram, "material.diffuse"), 0);
	terModel.Draw(shader);

	model = glm::mat4();
	model = glm::translate(model, glm::vec3(-100.0f, -15.0f, 50.1f));
	model = glm::scale(model, glm::vec3(0.7));
	model = glm::rotate(model, glm::radians(150.0f), glm::vec3(0, 1, 0));
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	glActiveTexture(GL_TEXTURE0);
	glUniform1i(glGetUniformLocation(myCustomShader.shaderProgram, "diffuseTexture"), 0);
	glUniform1i(glGetUniformLocation(shader.shaderProgram, "material.diffuse"), 0);
	treeModel.Draw(shader);

	GLfloat i;
	GLfloat j;
	GLfloat k = -50.0f;

	for (i = -100.0f; i < k; i += 10.0f) {
		for (j = 10.0f; j > -30; j -= 10.0f) {
			model = glm::mat4();
			model = glm::translate(model, glm::vec3(i, -7.0f, j));
			model = glm::scale(model, glm::vec3(0.2));
			glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
			glActiveTexture(GL_TEXTURE0);
			glUniform1i(glGetUniformLocation(myCustomShader.shaderProgram, "diffuseTexture"), 0);
			glUniform1i(glGetUniformLocation(shader.shaderProgram, "material.diffuse"), 0);
			grModel.Draw(shader);
		}
	}



	if (addSky)
	{
		initSkyBox();
		mySkyBox.Draw(skyboxShader, view, projection);
	}
}

GLuint depthMapTexture;
void initFramebuffer()
{

	glGenFramebuffers(1, &shadowMapFBO);

	//create depth texture for FBO
	glGenTextures(1, &depthMapTexture);
	glBindTexture(GL_TEXTURE_2D, depthMapTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMapTexture, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
GLuint planeVAO;

void RenderScene(gps::Shader &shader)
{
	// Floor
	glm::mat4 model;
	model = glm::scale(model, glm::vec3(5.0f));
	glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
	glBindVertexArray(planeVAO);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindVertexArray(0);


	
	
	

}




int main(int argc, const char * argv[]) {

	initOpenGLWindow();
	initOpenGLState();
	initModels();
	initShaders();

	myCustomShader.useShaderProgram();
	glUniform1i(glGetUniformLocation(myCustomShader.shaderProgram, "diffuseTexture"), 0);
	glUniform1i(glGetUniformLocation(myCustomShader.shaderProgram, "shadowMap"), 1);

	GLfloat planeVertices[] = {
		// Positions          // Normals         // Texture Coords
		25.0f, -1.0f, 25.0f, 0.0f, 1.0f, 0.0f, 25.0f, 0.0f,
		-25.0f, -1.0f, -25.0f, 0.0f, 1.0f, 0.0f, 0.0f, 25.0f,
		-25.0f, -1.0f, 25.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f,

		25.0f, -1.0f, 25.0f, 0.0f, 1.0f, 0.0f, 25.0f, 0.0f,
		25.0f, -1.0f, -25.0f, 0.0f, 1.0f, 0.0f, 25.0f, 25.0f,
		-25.0f, -1.0f, -25.0f, 0.0f, 1.0f, 0.0f, 0.0f, 25.0f
	};
	// Setup plane VAO
	GLuint planeVBO;
	glGenVertexArrays(1, &planeVAO);
	glGenBuffers(1, &planeVBO);
	glBindVertexArray(planeVAO);
	glBindBuffer(GL_ARRAY_BUFFER, planeVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(planeVertices), &planeVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(6 * sizeof(GLfloat)));
	glBindVertexArray(0);


	GLfloat skyVertices[] = {
		// Positions          // Normals         // Texture Coords
		30.0f, -10.0f, 30.0f, 10.0f, 10.0f, 10.0f, 25.0f, 0.0f,
		-30.0f, -10.0f, -30.0f, 10.0f, 10.0f, 10.0f, 0.0f, 25.0f,
		-30.0f, -10.0f, 30.0f, 10.0f, 10.0f, 10.0f, 0.0f, 0.0f,

		30.0f, -10.0f, 30.0f, 10.0f, 10.0f, 10.0f, 25.0f, 0.0f,
		30.0f, -10.0f, -30.0f, 10.0f, 10.0f, 10.0f, 25.0f, 25.0f,
		-30.0f, -10.0f, -30.0f, 10.0f, 10.0f, 10.0f, 0.0f, 25.0f
	};
	

	


	// Load textures
	GLuint woodTexture = ReadTextureFromFile("textures/grs2.jpg");
	//GLuint woodTexture = ReadTextureFromFile("textures/sky.bmp");
	GLuint forestTexture = ReadTextureFromFile("textures/sky.bmp");


	//for shadows
	initFramebuffer();

	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);

	while (!glfwWindowShouldClose(glWindow)) {
		glm::mat4 lightView = glm::lookAt(lightPos, glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		const GLfloat near_plane = 1.0f, far_plane = 7.5f;
		glm::mat4 lightProjection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, near_plane, far_plane);
		glm::mat4 lightSpaceTrMatrix = lightProjection * lightView;


		myShadowShader.useShaderProgram();
		glUniformMatrix4fv(glGetUniformLocation(myShadowShader.shaderProgram, "lightSpaceMatrix"), 1, GL_FALSE, glm::value_ptr(lightSpaceTrMatrix));
		
		glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
		glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
		glClear(GL_DEPTH_BUFFER_BIT);
		glCullFace(GL_FRONT);

		renderScene(myShadowShader);
		RenderScene(myShadowShader);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
		GLfloat borderColor[] = { 1.0, 1.0, 1.0, 1.0 };
		glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
		glCullFace(GL_BACK); // don't forget to reset original culling face

		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		// 2. Render scene as normal 
		glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		myCustomShader.useShaderProgram();

		//initAuxUnif(myCustomShader, lightPos);
		initUniformsLight(myCustomShader);
		//glUniform3fv(glGetUniformLocation(myCustomShader.shaderProgram, "dirLight.position"), 1, &lightPos[0]);
		//glUniform3fv(glGetUniformLocation(shader.shaderProgram, "lightPos"), 1, &lightPos[0]);
		//glUniform3fv(glGetUniformLocation(myCustomShader.shaderProgram, "viewPos"), 1, &myCamera.Position[0]);
		glUniformMatrix4fv(glGetUniformLocation(myCustomShader.shaderProgram, "lightSpaceMatrix"), 1, GL_FALSE, glm::value_ptr(lightSpaceTrMatrix));


		//renderScene(myCustomShader);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, depthMapTexture);
		renderScene(myCustomShader);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, woodTexture);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, depthMapTexture);
		RenderScene(myCustomShader);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, forestTexture);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, depthMapTexture);
		renderBackground(myCustomShader);




		// Render Depth map to quad
		debugDepthQuad.useShaderProgram();
		glUniform1f(glGetUniformLocation(debugDepthQuad.shaderProgram, "near_plane"), near_plane);
		glUniform1f(glGetUniformLocation(debugDepthQuad.shaderProgram, "far_plane"), far_plane);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, depthMapTexture);
		//RenderQuad();

		glfwPollEvents();
		glfwSwapBuffers(glWindow);
	}

	//close GL context and any other GLFW resources
	glfwTerminate();

	return 0;
}
