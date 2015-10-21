// Author: Paul F. Ascenzi Jr.

#include <iostream>
#include <cmath>
#include <vector>
#include <algorithm>    // std::max
using namespace std;

// GLEW
#define GLEW_STATIC
#include <GL/glew.h>

// GLFW
#include <GLFW/glfw3.h>

// Other Libs
#include <SOIL.h>
// GLM Mathematics
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// Other includes
#include "Camera.h"
#include "Shader.h"

//for convience
#define FOR(q,n) for(int q=0;q<n;q++)

// Function prototypes for callbacks
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void do_movement();

// Window dimensions
const GLuint WIDTH = 1200, HEIGHT = 600;

// Camera Intialization
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));


//  Globals to be used to transform the boxes
glm::vec3 boxScale		= glm::vec3(1.0f, 1.0f,  1.0f);
glm::vec3 boxTranslate    = glm::vec3(0.0f, 0.0f,  0.0f);
glm::vec3 rotationRate    = glm::vec3(0.01f, 0.01f,  0.01f);

// Globals used to increase rate of rotation
float alpha = 0.0f, beta = 0.0f, gamma = 0.0f;


GLfloat yaw    = -90.0f;	// Yaw is initialized to -90.0 degrees since a yaw of 0.0 results in a direction vector pointing to the right (due to how Eular angles work) so we initially rotate a bit to the left.
GLfloat pitch  =  0.0f;
GLfloat lastX  =  WIDTH  / 2.0;
GLfloat lastY  =  HEIGHT / 2.0;
GLfloat fov =  45.0f;
bool keys[1024];

// Deltatime
GLfloat deltaTime = 0.0f;	// Time between current frame and last frame
GLfloat lastFrame = 0.0f;  	// Time of last frame

// The MAIN function, from here we start the application and run the game loop
int main()
{
	// Init GLFW
	glfwInit();
	// Set all the required options for GLFW
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

	// Create a GLFWwindow object that we can use for GLFW's functions
	GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "LearnOpenGL", nullptr, nullptr);
	glfwMakeContextCurrent(window);

	// Set the required callback functions
	glfwSetKeyCallback(window, key_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);

	// GLFW Options
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	// Set this to true so GLEW knows to use a modern approach to retrieving function pointers and extensions
	glewExperimental = GL_TRUE;
	// Initialize GLEW to setup the OpenGL Function pointers
	glewInit();

	// Define the viewport dimensions
	glViewport(0, 0, WIDTH, HEIGHT);

	glEnable(GL_DEPTH_TEST);


	// Build and compile our shader program
	Shader ourShader("shaders/advanced.vs", "shaders/advanced.frag");


	//Load the Height Map and force 1 channel (so you can use RGB images as well)
	//  The values can be recast as integers and then used for the y values of the 
	//  heightmap.  The values range from [0,255].
	int ht_width, ht_height, ht_channels;
	unsigned char *ht_map = SOIL_load_image
		(
		"textures/hflab4.jpg",
		&ht_width, &ht_height, &ht_channels,
		SOIL_LOAD_L
		);

	// Set to 1 since 8bit intensity map
	unsigned int ptr_inc = 1;
	unsigned int row_step = ptr_inc * ht_width;

	vector<vector<glm::vec3>> vVertexData(ht_height, vector<glm::vec3>(ht_width));
	vector<vector<glm::vec2>> vCoordsData(ht_height, vector<glm::vec2>(ht_width));
	vector<GLfloat> htData;
	vector<GLfloat> htTriangles;

	
	FOR(i, ht_height)
	{
		FOR(j, ht_width)
		{
			GLfloat fScaleC = GLfloat(j)/GLfloat(ht_width-1);
			GLfloat fScaleR = GLfloat(i)/GLfloat(ht_height-1);
			GLfloat fVertexHeight = GLfloat(*(ht_map + row_step * i + j * ptr_inc))/255.0f;
			htData.push_back((fScaleC - 0.5f)*2);
			htData.push_back(fVertexHeight);
			htData.push_back((fScaleR - 0.5f)*2);
			htData.push_back(fScaleC);
			htData.push_back(fScaleR);
			//cout << (fScaleC -0.5f)*2 << " " << fVertexHeight << " " << (fScaleR -0.5f)*2 << " " << fScaleC << " " << fScaleR << endl;
			vVertexData[i][j] = glm::vec3((fScaleC*2.0f - 1.0f), -fVertexHeight/2-.5, (fScaleR*2.0f - 1.0f));
			vCoordsData[i][j] = glm::vec2(fScaleC, fScaleR);
			/*htData.push_back(vVertexData[i][j].x);
			htData.push_back(vVertexData[i][j].y);
			htData.push_back(vVertexData[i][j].z);
			htData.push_back(vCoordsData[i][j].x);
			htData.push_back(vCoordsData[i][j].y);*/
			//cout << htData.back() << endl;
			//cout << vVertexData[i][j].x << " " << vVertexData[i][j].y << " " << vVertexData[i][j].z << endl;
			//cout << vCoordsData[i][j].x << " " << vCoordsData[i][j].y << endl;
		}
	}

	// Generate the triangles
	FOR(i, ht_height-1)
	{
		FOR(j, ht_width-1)
		{
			// Triangle1 Part1
			htTriangles.push_back(vVertexData[i][j].x);
			htTriangles.push_back(vVertexData[i][j].y);
			htTriangles.push_back(vVertexData[i][j].z);
			htTriangles.push_back(vCoordsData[i][j].x);
			htTriangles.push_back(vCoordsData[i][j].y);
			// Part2
			htTriangles.push_back(vVertexData[i+1][j].x);
			htTriangles.push_back(vVertexData[i+1][j].y);
			htTriangles.push_back(vVertexData[i+1][j].z);
			htTriangles.push_back(vCoordsData[i+1][j].x);
			htTriangles.push_back(vCoordsData[i+1][j].y);
			// Part3
			htTriangles.push_back(vVertexData[i+1][j+1].x);
			htTriangles.push_back(vVertexData[i+1][j+1].y);
			htTriangles.push_back(vVertexData[i+1][j+1].z);
			htTriangles.push_back(vCoordsData[i+1][j+1].x);
			htTriangles.push_back(vCoordsData[i+1][j+1].y);

			// Triangle2 Part1
			htTriangles.push_back(vVertexData[i+1][j+1].x);
			htTriangles.push_back(vVertexData[i+1][j+1].y);
			htTriangles.push_back(vVertexData[i+1][j+1].z);
			htTriangles.push_back(vCoordsData[i+1][j+1].x);
			htTriangles.push_back(vCoordsData[i+1][j+1].y);
			// Part2
			htTriangles.push_back(vVertexData[i][j+1].x);
			htTriangles.push_back(vVertexData[i][j+1].y);
			htTriangles.push_back(vVertexData[i][j+1].z);
			htTriangles.push_back(vCoordsData[i][j+1].x);
			htTriangles.push_back(vCoordsData[i][j+1].y);
			// Part3
			htTriangles.push_back(vVertexData[i][j].x);
			htTriangles.push_back(vVertexData[i][j].y);
			htTriangles.push_back(vVertexData[i][j].z);
			htTriangles.push_back(vCoordsData[i][j].x);
			htTriangles.push_back(vCoordsData[i][j].y);
		}
	}

	GLuint VBOht, VAOht;
	glGenVertexArrays(1, &VAOht);
	glGenBuffers(1, &VBOht);
	// 2. Bind Vertex Array Object
	glBindVertexArray(VAOht);
	//  Bind the Vertex Buffer
	glBindBuffer(GL_ARRAY_BUFFER, VBOht);

	// 3. Copy our vertices array in a vertex buffer for OpenGL to use
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*htTriangles.size(), &htTriangles[0], GL_STATIC_DRAW);

	// 4.  Position attribute for the 3D Position Coordinates and link to position 0
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(0);
	// 5.  TexCoord attribute for the 2d Texture Coordinates and link to position 2
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
	glEnableVertexAttribArray(2);
	// 6.  Unbind Vertex Array Object
	glBindVertexArray(0);

	// Vertices for front side of skycube
	//      3D Coordinates      Texture Coordinates
	//    x       y      z     s     t  
	GLfloat front_vertices[] = {
		-1.0f, -1.0f, -1.0f,  1.0f, 0.0f,
		1.0f, -1.0f, -1.0f,  0.0f, 0.0f,
		1.0f,  1.0f, -1.0f,  0.0f, 1.0f,
		1.0f,  1.0f, -1.0f,  0.0f, 1.0f,
		-1.0f,  1.0f, -1.0f,  1.0f, 1.0f,
		-1.0f, -1.0f, -1.0f,  1.0f, 0.0f
	};	// finished

	// Vertices for back side of skycube
	//      3D Coordinates      Texture Coordinates
	//    x       y      z     s     t  
	GLfloat back_vertices[] = {
		-1.0f, -1.0f, 1.0f,  0.0f, 0.0f,
		1.0f, -1.0f, 1.0f,  1.0f, 0.0f,
		1.0f,  1.0f, 1.0f,  1.0f, 1.0f,
		-1.0f,  -1.0f, 1.0f,  0.0f, 0.0f,
		-1.0f,  1.0f, 1.0f,  0.0f, 1.0f,
		1.0f, 1.0f, 1.0f,  1.0f, 1.0f
	};	// finished

	// Vertices for right side of skycube
	//      3D Coordinates      Texture Coordinates
	//    x       y      z     s     t  
	GLfloat right_vertices[] = {
		1.0f, 1.0f, -1.0f,  1.0f, 1.0f,
		1.0f, -1.0f, -1.0f,  1.0f, 0.0f,
		1.0f,  1.0f, 1.0f,  0.0f, 1.0f,
		1.0f,  -1.0f, -1.0f,  1.0f, 0.0f,
		1.0f,  -1.0f, 1.0f,  0.0f, 0.0f,
		1.0f, 1.0f, 1.0f,  0.0f, 1.0f
	};	// finished

	// Vertices for left side of skycube
	//      3D Coordinates      Texture Coordinates
	//    x       y      z     s     t  
	GLfloat left_vertices[] = {
		-1.0f, 1.0f, -1.0f,  0.0f, 1.0f,
		-1.0f, -1.0f, -1.0f,  0.0f, 0.0f,
		-1.0f,  -1.0f, 1.0f,  1.0f, 0.0f,
		-1.0f,  1.0f, -1.0f,  0.0f, 1.0f,
		-1.0f,  1.0f, 1.0f,  1.0f, 1.0f,
		-1.0f, -1.0f, 1.0f,  1.0f, 0.0f
	};	// finished

	// Vertices for bottom side of skycube
	//      3D Coordinates      Texture Coordinates
	//    x       y      z     s     t  
	GLfloat bottom_vertices[] = {
		-1.0f, -1.0f, -1.0f,  1.0f, 0.0f,	// left bot back
		1.0f, -1.0f, -1.0f,  1.0f, 1.0f,	// right bot back
		-1.0f,  -1.0f, 1.0f,  0.0f, 0.0f,	// left bot front
		-1.0f,  -1.0f, 1.0f,  0.0f, 0.0f,	// left bot front
		1.0f,  -1.0f, 1.0f,  0.0f, 1.0f,	// right bot front
		1.0f, -1.0f, -1.0f,  1.0f, 1.0f		// right bot back
	};	// not finished

	// Vertices for top side of skycube
	//      3D Coordinates      Texture Coordinates
	//    x       y      z     s     t  
	GLfloat top_vertices[] = {
		-1.0f, 1.0f, -1.0f,  1.0f, 1.0f,
		-1.0f, 1.0f, 1.0f,  0.0f, 1.0f,
		1.0f,  1.0f, -1.0f,  1.0f, 0.0f,
		1.0f,  1.0f, -1.0f,  1.0f, 0.0f,
		1.0f,  1.0f, 1.0f,  0.0f, 0.0f,
		-1.0f, 1.0f, 1.0f,  0.0f, 1.0f
	};	// finished


	// Set up vertex data for boxes in the center
	GLfloat vertices[] = {
		-0.5f, -0.5f, -0.5f,  0.0f, 0.0f,
		0.5f, -0.5f, -0.5f,  1.0f, 0.0f,
		0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
		0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
		-0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
		-0.5f, -0.5f, -0.5f,  0.0f, 0.0f,

		-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
		0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
		0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
		0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
		-0.5f,  0.5f,  0.5f,  0.0f, 1.0f,
		-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,

		-0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
		-0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
		-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
		-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
		-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
		-0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

		0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
		0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
		0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
		0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
		0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
		0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

		-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
		0.5f, -0.5f, -0.5f,  1.0f, 1.0f,
		0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
		0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
		-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
		-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,

		-0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
		0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
		0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
		0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
		-0.5f,  0.5f,  0.5f,  0.0f, 0.0f,
		-0.5f,  0.5f, -0.5f,  0.0f, 1.0f
	};

	// Initial Translation Coordinates for each of the Boxes
	glm::vec3 cubePositions[] = {
		glm::vec3( 0.0f,  0.0f,  0.0f),
		glm::vec3( 2.0f,  5.0f, -15.0f),
		glm::vec3(-1.5f, -2.2f, -2.5f),
		glm::vec3(-3.8f, -2.0f, -12.3f),
		glm::vec3( 2.4f, -0.4f, -3.5f),
		glm::vec3(-1.7f,  3.0f, -7.5f),
		glm::vec3( 1.3f, -2.0f, -2.5f),
		glm::vec3( 1.5f,  2.0f, -2.5f),
		glm::vec3( 1.5f,  0.2f, -1.5f),
		glm::vec3(-1.3f,  1.0f, -1.5f)
	};

	//  1.  Create ID / Generate Buffers and for Vertex Buffer Object (VBO) and 
	//      Vertex Array Buffer (VAO)
	GLuint VBO, VAO;
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	// 2. Bind Vertex Array Object
	glBindVertexArray(VAO);
	//  Bind the Vertex Buffer
	glBindBuffer(GL_ARRAY_BUFFER, VBO);

	// 3. Copy our vertices array in a vertex buffer for OpenGL to use
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	// 4.  Position attribute for the 3D Position Coordinates and link to position 0
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(0);
	// 5.  TexCoord attribute for the 2d Texture Coordinates and link to position 2
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
	glEnableVertexAttribArray(2);

	// 6.  Unbind Vertex Array Object
	glBindVertexArray(0);


	// Create VBO and VAO for the front side of the skycube
	GLuint VBO_Front, VAO_Front;
	glGenVertexArrays(1, &VAO_Front);
	glGenBuffers(1, &VBO_Front);

	glBindVertexArray(VAO_Front);

	glBindBuffer(GL_ARRAY_BUFFER, VBO_Front);
	glBufferData(GL_ARRAY_BUFFER, sizeof(front_vertices), front_vertices, GL_STATIC_DRAW);

	// Position attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(0);
	// TexCoord attribute
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
	glEnableVertexAttribArray(2);

	glBindVertexArray(0); // Unbind VAO


	// Create VBO and VAO for the back side of the skycube
	GLuint VBO_Back, VAO_Back;
	glGenVertexArrays(1, &VAO_Back);
	glGenBuffers(1, &VBO_Back);

	glBindVertexArray(VAO_Back);

	glBindBuffer(GL_ARRAY_BUFFER, VBO_Back);
	glBufferData(GL_ARRAY_BUFFER, sizeof(back_vertices), back_vertices, GL_STATIC_DRAW);

	// Position attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(0);
	// TexCoord attribute
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
	glEnableVertexAttribArray(2);

	glBindVertexArray(0); // Unbind VAO


	// Create VBO and VAO for the left side of the skycube
	GLuint VBO_Left, VAO_Left;
	glGenVertexArrays(1, &VAO_Left);
	glGenBuffers(1, &VBO_Left);

	glBindVertexArray(VAO_Left);

	glBindBuffer(GL_ARRAY_BUFFER, VBO_Left);
	glBufferData(GL_ARRAY_BUFFER, sizeof(left_vertices), left_vertices, GL_STATIC_DRAW);

	// Position attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(0);
	// TexCoord attribute
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
	glEnableVertexAttribArray(2);

	glBindVertexArray(0); // Unbind VAO


	// Create VBO and VAO for the top side of the skycube
	GLuint VBO_Top, VAO_Top;
	glGenVertexArrays(1, &VAO_Top);
	glGenBuffers(1, &VBO_Top);

	glBindVertexArray(VAO_Top);

	glBindBuffer(GL_ARRAY_BUFFER, VBO_Top);
	glBufferData(GL_ARRAY_BUFFER, sizeof(top_vertices), top_vertices, GL_STATIC_DRAW);

	// Position attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(0);
	// TexCoord attribute
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
	glEnableVertexAttribArray(2);

	glBindVertexArray(0); // Unbind VAO


	
	GLuint VBO_Bottom, VAO_Bottom;
	glGenVertexArrays(1, &VAO_Bottom);
	glGenBuffers(1, &VBO_Bottom);

	glBindVertexArray(VAO_Bottom);

	glBindBuffer(GL_ARRAY_BUFFER, VBO_Bottom);
	glBufferData(GL_ARRAY_BUFFER, sizeof(bottom_vertices), bottom_vertices, GL_STATIC_DRAW);

	// Position attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(0);
	// TexCoord attribute
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
	glEnableVertexAttribArray(2);

	glBindVertexArray(0); // Unbind VAO


	// Create VBO and VAO for the front side of the skycube
	GLuint VBO_Right, VAO_Right;
	glGenVertexArrays(1, &VAO_Right);
	glGenBuffers(1, &VBO_Right);

	glBindVertexArray(VAO_Right);

	glBindBuffer(GL_ARRAY_BUFFER, VBO_Right);
	glBufferData(GL_ARRAY_BUFFER, sizeof(right_vertices), right_vertices, GL_STATIC_DRAW);

	// Position attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(0);
	// TexCoord attribute
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
	glEnableVertexAttribArray(2);

	glBindVertexArray(0); // Unbind VAO


	// Load and create a texture 
	GLuint texture1, texture2, // boxes
		texture3,	// front
		texture4,	// back
		texture5,	// left
		texture6,	// right
		texture7,	// top
		texture8,	// bottom
		texture9;	// heightmap
	// ====================
	// Texture 1
	// ====================
	glGenTextures(1, &texture1);
	glBindTexture(GL_TEXTURE_2D, texture1); // All upcoming GL_TEXTURE_2D operations now have effect on our texture object
	// Set our texture parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);	// Set texture wrapping to GL_CLAMP_TO_EDGE
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	// Set texture filtering
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	// Load, create texture and generate mipmaps
	int width, height;
	unsigned char* image = SOIL_load_image("textures/container.jpg", &width, &height, 0, SOIL_LOAD_RGB);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
	glGenerateMipmap(GL_TEXTURE_2D);
	SOIL_free_image_data(image);
	glBindTexture(GL_TEXTURE_2D, 0); // Unbind texture when done, so we won't accidentily mess up our texture.
	// ===================
	// Texture 2
	// ===================
	glGenTextures(1, &texture2);
	glBindTexture(GL_TEXTURE_2D, texture2);
	// Set our texture parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	// Set texture filtering
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	// Load, create texture and generate mipmaps
	image = SOIL_load_image("textures/psulogo.png", &width, &height, 0, SOIL_LOAD_RGB);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
	glGenerateMipmap(GL_TEXTURE_2D);
	SOIL_free_image_data(image);
	glBindTexture(GL_TEXTURE_2D, 0);

	// ===================
	// Box Textures Front
	// ===================
	glGenTextures(1, &texture3);
	glBindTexture(GL_TEXTURE_2D, texture3);
	// Set our texture parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	// Set texture filtering
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	// Load, create texture and generate mipmaps
	image = SOIL_load_image("skybox/front.jpg", &width, &height, 0, SOIL_LOAD_RGB);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
	glGenerateMipmap(GL_TEXTURE_2D);
	SOIL_free_image_data(image);
	glBindTexture(GL_TEXTURE_2D, 0);

	// ===================
	// Box Textures Back
	// ===================
	glGenTextures(1, &texture4);
	glBindTexture(GL_TEXTURE_2D, texture4);
	// Set our texture parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	// Set texture filtering
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	// Load, create texture and generate mipmaps
	image = SOIL_load_image("skybox/back.jpg", &width, &height, 0, SOIL_LOAD_RGB);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
	glGenerateMipmap(GL_TEXTURE_2D);
	SOIL_free_image_data(image);
	glBindTexture(GL_TEXTURE_2D, 0);

	// ===================
	// Box Textures Left
	// ===================
	glGenTextures(1, &texture5);
	glBindTexture(GL_TEXTURE_2D, texture5);
	// Set our texture parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	// Set texture filtering
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	// Load, create texture and generate mipmaps
	image = SOIL_load_image("skybox/left.jpg", &width, &height, 0, SOIL_LOAD_RGB);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
	glGenerateMipmap(GL_TEXTURE_2D);
	SOIL_free_image_data(image);
	glBindTexture(GL_TEXTURE_2D, 0);

	// ===================
	// Box Textures Right
	// ===================
	glGenTextures(1, &texture6);
	glBindTexture(GL_TEXTURE_2D, texture6);
	// Set our texture parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	// Set texture filtering
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	// Load, create texture and generate mipmaps
	image = SOIL_load_image("skybox/right.jpg", &width, &height, 0, SOIL_LOAD_RGB);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
	glGenerateMipmap(GL_TEXTURE_2D);
	SOIL_free_image_data(image);
	glBindTexture(GL_TEXTURE_2D, 0);

	// ===================
	// Box Textures Top
	// ===================
	glGenTextures(1, &texture7);
	glBindTexture(GL_TEXTURE_2D, texture7);
	// Set our texture parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	// Set texture filtering
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	// Load, create texture and generate mipmaps
	image = SOIL_load_image("skybox/top.jpg", &width, &height, 0, SOIL_LOAD_RGB);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
	glGenerateMipmap(GL_TEXTURE_2D);
	SOIL_free_image_data(image);
	glBindTexture(GL_TEXTURE_2D, 0);

	// ===================
	// Box Textures Bottom
	// ===================
	glGenTextures(1, &texture8);
	glBindTexture(GL_TEXTURE_2D, texture8);
	// Set our texture parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	// Set texture filtering
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	// Load, create texture and generate mipmaps
	image = SOIL_load_image("skybox/bottom.jpg", &width, &height, 0, SOIL_LOAD_RGB);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
	glGenerateMipmap(GL_TEXTURE_2D);
	SOIL_free_image_data(image);
	glBindTexture(GL_TEXTURE_2D, 0);

	// ===================
	// Heightmap Texture
	// ===================
	

	// Game loop
	while (!glfwWindowShouldClose(window))
	{
		// Calculate deltatime of current frame
		GLfloat currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;
		// Check if any events have been activiated (key pressed, mouse moved etc.) and call corresponding response functions
		glfwPollEvents();
		do_movement();

		// Render
		// Clear the colorbuffer
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Camera/View transformation
		// Create View Matrix
		glm::mat4 view;
		view = camera.GetViewMatrix();
		// Create Projection Matrix  
		glm::mat4 projection;
		projection = glm::perspective(fov, (GLfloat)WIDTH/(GLfloat)HEIGHT, 0.1f, 100.0f);  


		// Get the uniform locations for model, view, and project matrix in the shader program
		GLint modelLoc = glGetUniformLocation(ourShader.Program, "model");
		GLint viewLoc = glGetUniformLocation(ourShader.Program, "view");
		GLint projLoc = glGetUniformLocation(ourShader.Program, "projection");

		// Pass the matrice pointers to the shader
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));


		// Activate shader
		ourShader.Use();

		//  Load Textures for the Front of the Skycube
		// 1.  Activate the texture unit GL_TEXTURE0 so it can be used 
		//         (and is the one you are currently loading into)
		glActiveTexture(GL_TEXTURE0);
		// 2.  Bind the texture3 object to use in the GL_TEXTURE0 texture unit
		glBindTexture(GL_TEXTURE_2D, texture3);
		// 3.  Send the texture information to the shader variable 'ourTexture1' in the fragment shader
		glUniform1i(glGetUniformLocation(ourShader.Program, "ourTexture1"), 0);
		// 4.  Activate a second texture unit to use another texture at the same time
		glActiveTexture(GL_TEXTURE1);
		// 5.  Bind texture3 again to texture unit GL_TEXTURE1.  This is only necessary since the shader
		//      takes two textures and mixes them together. 
		glBindTexture(GL_TEXTURE_2D, texture3);
		// 6.  Send the texture information to the shader variable `ourTexture2'
		glUniform1i(glGetUniformLocation(ourShader.Program, "ourTexture2"), 1);


		// Draw the front side of the skybox
		// 1. Bind the vertex array 
		glBindVertexArray(VAO_Front);
		// 2. Create the Model Matrix
		glm::mat4 model ;
		// 4.  Scale the model matrix by 50.0f (f is to make it a float)
		model = glm::scale(model, glm::vec3(50.0f,50.0f,50.0f));
		// 6.  Send the matrix pointer of the model matrix to the shader
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		// 7.  Draw the two triangles consisting of 6 sides.
		glDrawArrays(GL_TRIANGLES, 0, 6);
		// 8.  Unbind the vertex array
		glBindVertexArray(0);


		//  Load Textures for the Back of the Skycube
		// 1.  Activate the texture unit GL_TEXTURE0 so it can be used 
		//         (and is the one you are currently loading into)
		glActiveTexture(GL_TEXTURE0);
		// 2.  Bind the texture3 object to use in the GL_TEXTURE0 texture unit
		glBindTexture(GL_TEXTURE_2D, texture4);
		// 3.  Send the texture information to the shader variable 'ourTexture1' in the fragment shader
		glUniform1i(glGetUniformLocation(ourShader.Program, "ourTexture1"), 0);
		// 4.  Activate a second texture unit to use another texture at the same time
		glActiveTexture(GL_TEXTURE1);
		// 5.  Bind texture3 again to texture unit GL_TEXTURE1.  This is only necessary since the shader
		//      takes two textures and mixes them together. 
		glBindTexture(GL_TEXTURE_2D, texture4);
		// 6.  Send the texture information to the shader variable `ourTexture2'
		glUniform1i(glGetUniformLocation(ourShader.Program, "ourTexture2"), 1);

		// Draw the back side of the skybox
		// 1. Bind the vertex array 
		glBindVertexArray(VAO_Back);
		// 2. Create the Model Matrix
		glm::mat4 model2 ;
		// 4.  Scale the model matrix by 50.0f (f is to make it a float)
		model2 = glm::scale(model2, glm::vec3(50.0f,50.0f,50.0f));
		// 6.  Send the matrix pointer of the model matrix to the shader
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model2));
		// 7.  Draw the two triangles consisting of 6 sides.
		glDrawArrays(GL_TRIANGLES, 0, 6);
		// 8.  Unbind the vertex array
		glBindVertexArray(0);


		//  Load Textures for the Left of the Skycube
		// 1.  Activate the texture unit GL_TEXTURE0 so it can be used 
		//         (and is the one you are currently loading into)
		glActiveTexture(GL_TEXTURE0);
		// 2.  Bind the texture3 object to use in the GL_TEXTURE0 texture unit
		glBindTexture(GL_TEXTURE_2D, texture5);
		// 3.  Send the texture information to the shader variable 'ourTexture1' in the fragment shader
		glUniform1i(glGetUniformLocation(ourShader.Program, "ourTexture1"), 0);
		// 4.  Activate a second texture unit to use another texture at the same time
		glActiveTexture(GL_TEXTURE1);
		// 5.  Bind texture3 again to texture unit GL_TEXTURE1.  This is only necessary since the shader
		//      takes two textures and mixes them together. 
		glBindTexture(GL_TEXTURE_2D, texture5);
		// 6.  Send the texture information to the shader variable `ourTexture2'
		glUniform1i(glGetUniformLocation(ourShader.Program, "ourTexture2"), 1);

		// Draw the left side of the skybox
		// 1. Bind the vertex array 
		glBindVertexArray(VAO_Left);
		// 2. Create the Model Matrix
		glm::mat4 model3 ;
		// 4.  Scale the model matrix by 50.0f (f is to make it a float)
		model3 = glm::scale(model3, glm::vec3(50.0f,50.0f,50.0f));
		// 6.  Send the matrix pointer of the model matrix to the shader
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model3));
		// 7.  Draw the two triangles consisting of 6 sides.
		glDrawArrays(GL_TRIANGLES, 0, 6);
		// 8.  Unbind the vertex array
		glBindVertexArray(0);


		//  Load Textures for the Right of the Skycube
		// 1.  Activate the texture unit GL_TEXTURE0 so it can be used 
		//         (and is the one you are currently loading into)
		glActiveTexture(GL_TEXTURE0);
		// 2.  Bind the texture3 object to use in the GL_TEXTURE0 texture unit
		glBindTexture(GL_TEXTURE_2D, texture6);
		// 3.  Send the texture information to the shader variable 'ourTexture1' in the fragment shader
		glUniform1i(glGetUniformLocation(ourShader.Program, "ourTexture1"), 0);
		// 4.  Activate a second texture unit to use another texture at the same time
		glActiveTexture(GL_TEXTURE1);
		// 5.  Bind texture3 again to texture unit GL_TEXTURE1.  This is only necessary since the shader
		//      takes two textures and mixes them together. 
		glBindTexture(GL_TEXTURE_2D, texture6);
		// 6.  Send the texture information to the shader variable `ourTexture2'
		glUniform1i(glGetUniformLocation(ourShader.Program, "ourTexture2"), 1);

		// Draw the right side of the skybox
		// 1. Bind the vertex array 
		glBindVertexArray(VAO_Right);
		// 2. Create the Model Matrix
		glm::mat4 model4 ;
		// 4.  Scale the model matrix by 50.0f (f is to make it a float)
		model4 = glm::scale(model4, glm::vec3(50.0f,50.0f,50.0f));
		// 6.  Send the matrix pointer of the model matrix to the shader
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model4));
		// 7.  Draw the two triangles consisting of 6 sides.
		glDrawArrays(GL_TRIANGLES, 0, 6);
		// 8.  Unbind the vertex array
		glBindVertexArray(0);


		//  Load Textures for the Top of the Skycube
		// 1.  Activate the texture unit GL_TEXTURE0 so it can be used 
		//         (and is the one you are currently loading into)
		glActiveTexture(GL_TEXTURE0);
		// 2.  Bind the texture3 object to use in the GL_TEXTURE0 texture unit
		glBindTexture(GL_TEXTURE_2D, texture7);
		// 3.  Send the texture information to the shader variable 'ourTexture1' in the fragment shader
		glUniform1i(glGetUniformLocation(ourShader.Program, "ourTexture1"), 0);
		// 4.  Activate a second texture unit to use another texture at the same time
		glActiveTexture(GL_TEXTURE1);
		// 5.  Bind texture3 again to texture unit GL_TEXTURE1.  This is only necessary since the shader
		//      takes two textures and mixes them together. 
		glBindTexture(GL_TEXTURE_2D, texture7);
		// 6.  Send the texture information to the shader variable `ourTexture2'
		glUniform1i(glGetUniformLocation(ourShader.Program, "ourTexture2"), 1);

		// Draw the top side of the skybox
		// 1. Bind the vertex array 
		glBindVertexArray(VAO_Top);
		// 2. Create the Model Matrix
		glm::mat4 model5 ;
		// 4.  Scale the model matrix by 50.0f (f is to make it a float)
		model5 = glm::scale(model5, glm::vec3(50.0f,50.0f,50.0f));
		// 6.  Send the matrix pointer of the model matrix to the shader
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model5));
		// 7.  Draw the two triangles consisting of 6 sides.
		glDrawArrays(GL_TRIANGLES, 0, 6);
		// 8.  Unbind the vertex array
		glBindVertexArray(0);

		////  Load Textures for the Bottom of the Skycube
		//// 1.  Activate the texture unit GL_TEXTURE0 so it can be used 
		////         (and is the one you are currently loading into)
		//glActiveTexture(GL_TEXTURE0);
		//// 2.  Bind the texture3 object to use in the GL_TEXTURE0 texture unit
		//glBindTexture(GL_TEXTURE_2D, texture8);
		//// 3.  Send the texture information to the shader variable 'ourTexture1' in the fragment shader
		//glUniform1i(glGetUniformLocation(ourShader.Program, "ourTexture1"), 0);
		//// 4.  Activate a second texture unit to use another texture at the same time
		//glActiveTexture(GL_TEXTURE1);
		//// 5.  Bind texture3 again to texture unit GL_TEXTURE1.  This is only necessary since the shader
		////      takes two textures and mixes them together. 
		//glBindTexture(GL_TEXTURE_2D, texture8);
		//// 6.  Send the texture information to the shader variable `ourTexture2'
		//glUniform1i(glGetUniformLocation(ourShader.Program, "ourTexture2"), 1);

		//// Draw the bottom side of the skybox
		//// 1. Bind the vertex array 
		//glBindVertexArray(VAO_Bottom);
		//// 2. Create the Model Matrix
		//glm::mat4 model6 ;
		//// 4.  Scale the model matrix by 50.0f (f is to make it a float)
		//model6 = glm::scale(model6, glm::vec3(50.0f,50.0f,50.0f));
		//// 6.  Send the matrix pointer of the model matrix to the shader
		//glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model6));
		//// 7.  Draw the two triangles consisting of 6 sides.
		//glDrawArrays(GL_TRIANGLES, 0, 6);
		//// 8.  Unbind the vertex array
		//glBindVertexArray(0);

		//  Load Textures for the Bottom of the Skycube
		// 1.  Activate the texture unit GL_TEXTURE0 so it can be used 
		//         (and is the one you are currently loading into)
		glActiveTexture(GL_TEXTURE0);
		// 2.  Bind the texture3 object to use in the GL_TEXTURE0 texture unit
		glBindTexture(GL_TEXTURE_2D, texture8);
		// 3.  Send the texture information to the shader variable 'ourTexture1' in the fragment shader
		glUniform1i(glGetUniformLocation(ourShader.Program, "ourTexture1"), 0);
		// 4.  Activate a second texture unit to use another texture at the same time
		glActiveTexture(GL_TEXTURE1);
		// 5.  Bind texture3 again to texture unit GL_TEXTURE1.  This is only necessary since the shader
		//      takes two textures and mixes them together. 
		glBindTexture(GL_TEXTURE_2D, texture8);
		// 6.  Send the texture information to the shader variable `ourTexture2'
		glUniform1i(glGetUniformLocation(ourShader.Program, "ourTexture2"), 1);
		// Draw the height map
		glBindVertexArray(VAOht);

		glm::mat4 model7 ;
		// 4.  Scale the model matrix by 50.0f (f is to make it a float)
		model7 = glm::scale(model7, glm::vec3(50.0f,50.0,50.0f));
		// 6.  Send the matrix pointer of the model matrix to the shader
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model7));

		glDrawArrays(GL_TRIANGLES, 0, 5*3*2*(ht_height-1)*(ht_width-1));
		glBindVertexArray(0);


		// Bind Textures using texture units
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture1);
		glUniform1i(glGetUniformLocation(ourShader.Program, "ourTexture1"), 0);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, texture2);
		glUniform1i(glGetUniformLocation(ourShader.Program, "ourTexture2"), 1);


		//  Draw each of the Boxes in the center
		glBindVertexArray(VAO);
		for (GLuint i = 0; i < 10; i++)
		{
			// Calculate the model matrix for each object and pass it to shader before drawing
			glm::mat4 model, angle;

			// Make the boxes transform in time
			model = glm::translate(model, boxTranslate);
			model = glm::translate(model, cubePositions[i]);
			model = glm::rotate(model,(GLfloat)glfwGetTime() * 0.5f, rotationRate);
			model = glm::rotate(model,(GLfloat)glfwGetTime() * alpha, glm::vec3(1.0f, 0.0f, 0.0f));
			model = glm::rotate(model,(GLfloat)glfwGetTime() * beta, glm::vec3(0.0f, 1.0f, 0.0f));
			model = glm::rotate(model,(GLfloat)glfwGetTime() * gamma, glm::vec3(0.0f, 0.0f, 1.0f));
			model = glm::scale(model, boxScale);


			glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

			glDrawArrays(GL_TRIANGLES, 0, 36);
		}
		glBindVertexArray(0);


		// Swap the screen buffers
		glfwSwapBuffers(window);
	}
	// Properly de-allocate all resources once they've outlived their purpose
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);
	glDeleteVertexArrays(1, &VAO_Front);
	glDeleteBuffers(1, &VBO_Front);
	glDeleteVertexArrays(1, &VAO_Back);
	glDeleteBuffers(1, &VBO_Back);
	glDeleteVertexArrays(1, &VAO_Left);
	glDeleteBuffers(1, &VBO_Left);
	glDeleteVertexArrays(1, &VAO_Right);
	glDeleteBuffers(1, &VBO_Right);
	glDeleteVertexArrays(1, &VAO_Top);
	glDeleteBuffers(1, &VBO_Top);
	glDeleteVertexArrays(1, &VAO_Bottom);
	glDeleteBuffers(1, &VBO_Bottom);
	glDeleteBuffers(1, &VAOht);
	glDeleteBuffers(1, &VBOht);

	// Terminate GLFW, clearing any resources allocated by GLFW.
	glfwTerminate();
	return 0;
}


#pragma region "User Input"
// Is called whenever a key is pressed/released via GLFW
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);
	if (key >= 0 && key < 1024)
	{
		if (action == GLFW_PRESS)
			keys[key] = true;
		else if (action == GLFW_RELEASE)
			keys[key] = false;
	}
}

void do_movement()
{
	// Camera controls
	GLfloat cameraSpeed = 5.0f * deltaTime;
	if(keys[GLFW_KEY_W])
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if(keys[GLFW_KEY_S])
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if(keys[GLFW_KEY_A])
        camera.ProcessKeyboard(LEFT, deltaTime);
    if(keys[GLFW_KEY_D])
        camera.ProcessKeyboard(RIGHT, deltaTime);
	if(keys[GLFW_KEY_Q])
        camera.ProcessKeyboard(UP, deltaTime);
	if(keys[GLFW_KEY_E])
        camera.ProcessKeyboard(DOWN, deltaTime);
	if (keys[GLFW_KEY_P])
		int save_result = SOIL_save_screenshot
		(
		"awesomenessity.bmp",
		SOIL_SAVE_TYPE_BMP,
		0, 0, WIDTH, HEIGHT
		);

	// Define more keys here

	// Increase Rotation on X Axis
	if (keys[GLFW_KEY_U])
		alpha += 0.01f;
	// Decrease Rotation on X Axis
	if (keys[GLFW_KEY_J])
		alpha -= 0.01f;
	// Increase Rotation on Y Axis
	if (keys[GLFW_KEY_I])
		beta += 0.01f;
	// Descrease Rotation on Y Axis
	if (keys[GLFW_KEY_K])
		beta -= 0.01f;
	//Increase Rotation on Z Axis
	if (keys[GLFW_KEY_O])
		gamma += 0.01f;
	//Decrease Rotation on Z Axis
	if (keys[GLFW_KEY_L])
		gamma -= 0.01f;

	//Reset Rotation and Scale on all Axises
	if (keys[GLFW_KEY_R]) {
		alpha = 0.0f;
		beta = 0.0f;
		gamma = 0.0f;
		boxScale = glm::vec3(1.0f, 1.0f, 1.0f);
	}

	// Increase Scale on X Axis
	if (keys[GLFW_KEY_U] & keys[GLFW_KEY_RIGHT_SHIFT])
		boxScale = boxScale + glm::vec3(0.01f, 0.0f, 0.0f);
	// Decrease Scale on X Axis
	if (keys[GLFW_KEY_J] & keys[GLFW_KEY_RIGHT_SHIFT])
		boxScale = boxScale - glm::vec3(0.01f, 0.0f, 0.0f);
	// Increase Scale on Y Axis
	if (keys[GLFW_KEY_I] & keys[GLFW_KEY_RIGHT_SHIFT])
		boxScale = boxScale + glm::vec3(0.0f, 0.01f, 0.0f);
	// Descrease Scale on Y Axis
	if (keys[GLFW_KEY_K] & keys[GLFW_KEY_RIGHT_SHIFT])
		boxScale = boxScale - glm::vec3(0.0f, 0.01f, 0.0f);
	//Increase Scale on Z Axis
	if (keys[GLFW_KEY_O] & keys[GLFW_KEY_RIGHT_SHIFT])
		boxScale = boxScale + glm::vec3(0.0f, 0.0f, 0.01f);
	//Decrease Scale on Z Axis
	if (keys[GLFW_KEY_L] & keys[GLFW_KEY_RIGHT_SHIFT])
		boxScale = boxScale - glm::vec3(0.0f, 0.0f, 0.01f);

	// Increase Translation on X Axis
	if (keys[GLFW_KEY_U] & keys[GLFW_KEY_RIGHT_CONTROL])
		boxTranslate = boxTranslate + glm::vec3(0.01f, 0.0f, 0.0f);
	// Decrease Translation on X Axis
	if (keys[GLFW_KEY_J] & keys[GLFW_KEY_RIGHT_CONTROL])
		boxTranslate = boxTranslate - glm::vec3(0.01f, 0.0f, 0.0f);
	// Increase Translation on Y Axis
	if (keys[GLFW_KEY_I] & keys[GLFW_KEY_RIGHT_CONTROL])
		boxTranslate = boxTranslate + glm::vec3(0.0f, 0.01f, 0.0f);
	// Descrease Translation on Y Axis
	if (keys[GLFW_KEY_K] & keys[GLFW_KEY_RIGHT_CONTROL])
		boxTranslate = boxTranslate - glm::vec3(0.0f, 0.01f, 0.0f);
	//Increase Translation on Z Axis
	if (keys[GLFW_KEY_O] & keys[GLFW_KEY_RIGHT_CONTROL])
		boxTranslate = boxTranslate + glm::vec3(0.0f, 0.0f, 0.01f);
	//Decrease Translation on Z Axis
	if (keys[GLFW_KEY_L] & keys[GLFW_KEY_RIGHT_CONTROL])
		boxTranslate = boxTranslate - glm::vec3(0.0f, 0.0f, 0.01f);
}

bool firstMouse = true;
void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	 if(firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    GLfloat xoffset = xpos - lastX;
    GLfloat yoffset = lastY - ypos; 
    
    lastX = xpos;
    lastY = ypos;

    camera.ProcessMouseMovement(xoffset, yoffset);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	if (fov >= 1.0f && fov <= 45.0f)
		fov -= yoffset;
	if (fov <= 1.0f)
		fov = 1.0f;
	if (fov >= 45.0f)
		fov = 45.0f;

}

#pragma endregion "User Input"
