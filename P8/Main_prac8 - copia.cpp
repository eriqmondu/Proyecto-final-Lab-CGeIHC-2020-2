#include <iostream>
#include <cmath>

// GLEW
#include <GL/glew.h>

// GLFW
#include <GLFW/glfw3.h>

// Librería para cargar las texturas
#include "stb_image.h"

// GLM Mathematics
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// Librería para carga de texturas a OpenGL
#include "SOIL2/SOIL2.h"

// Other includes
#include "Shader.h"
#include "Camera.h"
#include "Model.h"
#include "Texture.h"

// Prototipos de funciones
void KeyCallback(GLFWwindow *window, int key, int scancode, int action, int mode);
void MouseCallback(GLFWwindow *window, double xPos, double yPos);
void DoMovement(void);
void LoadTextures(void);
unsigned int generateTextures(char*, bool);

void animacion();

// Window dimensions
const GLuint WIDTH = 800, HEIGHT = 600;
int SCREEN_WIDTH, SCREEN_HEIGHT;

// Camera
Camera  camera(glm::vec3(0.0f, 0.0f, 3.0f));
GLfloat lastX = WIDTH / 2.0;
GLfloat lastY = HEIGHT / 2.0;
bool keys[1024];
bool firstMouse = true;
float rot = 0.0f;
float range = 0.0f;
float spotAngle = 0.0f;

// Light attributes
glm::vec3 lightPos(0.0f, -5.0f, 0.0f);
glm::vec3 PosIni(-95.0f, 1.0f, -45.0f);
bool active;

// Posición de las cuatro luces puntuales
glm::vec3 pointLightPositions[] = {
	glm::vec3(2.0f,  1.0f,  1.0f),
	glm::vec3(0.0f, 0.0f, 0.0f),
	glm::vec3(2.0f,  1.0f, 0.0f),
	glm::vec3(0.0f,  0.0f, 0.0f)
};

// Para habilitar / deshabilitar alguna luz
glm::vec3 LightP1; 

// Variables para mover objetos de manera interactiva
float movX, movY, movZ;

// Deltatime
GLfloat deltaTime = 0.0f;	// Time between current frame and last frame
GLfloat lastFrame = 0.0f;  	// Time of last frame

// Keyframes
float posX = PosIni.x, posY = PosIni.y, posZ = PosIni.z, rotRodIzq = 0, rotRodDer = 0, rotBraIzq = 0, rotBraDer = 0;

#define MAX_FRAMES 9
int i_max_steps = 40;
int i_curr_steps = 0;
typedef struct _frame
{
	//Variables para GUARDAR Key Frames
	float posX;		//Variable para PosicionX
	float posY;		//Variable para PosicionY
	float posZ;		//Variable para PosicionZ
	float incX;		//Variable para IncrementoX
	float incY;		//Variable para IncrementoY
	float incZ;		//Variable para IncrementoZ
	float rotRodIzq;
	float rotRodDer;
	float rotInc;
	float rotInc2;
	float rotBraIzq;
	float rotBraDer;
	float rotInc3;
	float rotInc4;

}FRAME;

FRAME KeyFrame[MAX_FRAMES];
int FrameIndex = 0;			//introducir datos
bool play = false;
int playIndex = 0;

void saveFrame(void)
{

	printf("frameindex %d\n", FrameIndex);

	KeyFrame[FrameIndex].posX = posX;
	KeyFrame[FrameIndex].posY = posY;
	KeyFrame[FrameIndex].posZ = posZ;

	KeyFrame[FrameIndex].rotRodIzq = rotRodIzq;
	KeyFrame[FrameIndex].rotRodDer = rotRodDer;

	KeyFrame[FrameIndex].rotBraIzq = rotBraIzq;
	KeyFrame[FrameIndex].rotBraDer = rotBraDer;


	FrameIndex++;
}

void resetElements(void)
{
	posX = KeyFrame[0].posX;
	posY = KeyFrame[0].posY;
	posZ = KeyFrame[0].posZ;

	rotRodIzq = KeyFrame[0].rotRodIzq;
	rotRodDer = KeyFrame[0].rotRodDer;

	rotBraIzq = KeyFrame[0].rotBraIzq;
	rotBraDer = KeyFrame[0].rotBraDer;

}

void interpolation(void)
{

	KeyFrame[playIndex].incX = (KeyFrame[playIndex + 1].posX - KeyFrame[playIndex].posX) / i_max_steps;
	KeyFrame[playIndex].incY = (KeyFrame[playIndex + 1].posY - KeyFrame[playIndex].posY) / i_max_steps;
	KeyFrame[playIndex].incZ = (KeyFrame[playIndex + 1].posZ - KeyFrame[playIndex].posZ) / i_max_steps;

	KeyFrame[playIndex].rotInc = (KeyFrame[playIndex + 1].rotRodIzq - KeyFrame[playIndex].rotRodIzq) / i_max_steps;
	KeyFrame[playIndex].rotInc2 = (KeyFrame[playIndex + 1].rotRodDer - KeyFrame[playIndex].rotRodDer) / i_max_steps;

	KeyFrame[playIndex].rotInc3 = (KeyFrame[playIndex + 1].rotBraIzq - KeyFrame[playIndex].rotBraIzq) / i_max_steps;
	KeyFrame[playIndex].rotInc4 = (KeyFrame[playIndex + 1].rotBraDer - KeyFrame[playIndex].rotBraDer) / i_max_steps;

}



// Definición de texturas para primitivas
GLuint specular_map, 
	   texture0, 
	   texture1,
	   texture2,
	   texture3,
	   texture4, 
	   texture5,
	   texture6,
	   texture7,
	   texture8,
	   texture9;

unsigned int generateTextures(const char* filename, bool alfa) {
	unsigned int textureID;

	// set the texture wrapping parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	// set texture filtering parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST_MIPMAP_NEAREST);

	// load image, create texture and generate mipmaps
	int width, height, nrChannels;
	stbi_set_flip_vertically_on_load(true); // tell stb_image.h to flip loaded texture's on the y-axis.

	unsigned char* data = stbi_load(filename, &width, &height, &nrChannels, 0);
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_2D, textureID);
	if (data)
	{
		if (alfa)
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		else
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
		return textureID;
	}
	else
	{
		std::cout << "Failed to load texture" << std::endl;
		return 100;
	}

	stbi_image_free(data);
}

void LoadTextures() {
	specular_map = generateTextures("images/mapa_blanco.jpg", 0);
	texture0 = generateTextures("images/floor.jpg", 0);
	texture1 = generateTextures("images/tapete.jpg", 0);
	texture2 = generateTextures("images/pared.jpg", 0);
	texture3 = generateTextures("images/window.png", 1);
	texture4 = generateTextures("images/cuadro1.jpg", 0);
	texture5 = generateTextures("images/cuadro2.jpg", 0);
	texture6 = generateTextures("images/window_front.png", 1);
	texture7 = generateTextures("images/cj_mom.jpg", 0);
	texture8 = generateTextures("images/puerta_blanca.jpg", 0);
	texture9 = generateTextures("images/puerta_cafe.jpg", 0);
	//stbi_set_flip_vertically_on_load(false);
	glBindTexture(GL_TEXTURE_2D, 0);
}

int main()
{
	// Init GLFW
	glfwInit();
	// Set all the required options for GLFW
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
	glfwWindowHint(GLFW_SAMPLES, 4);

	// Create a GLFWwindow object that we can use for GLFW's functions
	GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Proyecto final", nullptr, nullptr);

	if (nullptr == window)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();

		return EXIT_FAILURE;
	}

	glfwMakeContextCurrent(window);
	glfwGetFramebufferSize(window, &SCREEN_WIDTH, &SCREEN_HEIGHT);

	// Set the required callback functions
	glfwSetKeyCallback(window, KeyCallback);
	glfwSetCursorPosCallback(window, MouseCallback);

	// GLFW Options
	//glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	// Set this to true so GLEW knows to use a modern approach to retrieving function pointers and extensions
	glewExperimental = GL_TRUE;
	// Initialize GLEW to setup the OpenGL Function pointers
	if (GLEW_OK != glewInit())
	{
		std::cout << "Failed to initialize GLEW" << std::endl;
		return EXIT_FAILURE;
	}

	// Define the viewport dimensions
	glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);


	// OpenGL options
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	Shader lightingShader("Shaders/lighting.vs", "Shaders/lighting.frag");
	Shader lampShader("Shaders/lamp.vs", "Shaders/lamp.frag");
	Shader modelLoading("Shaders/modelLoading.vs", "Shaders/modelLoading.frag");
	Shader SkyBoxshader("Shaders/SkyBox.vs", "Shaders/SkyBox.frag");
	
	// Modelos OBJ
	Model sillon((char*)"Models/Sofas/sillon.obj");
	Model mesa_centro((char*)"Models/Mesa/table.obj");
	Model planta_mesa_centro((char*)"Models/Planta/planta.obj");
	Model sofa((char*)"Models/Sofas/sofa_sencillo.obj");
	Model cortina((char*)"Models/cortina.obj");
	Model tv((char*)"Models/TV/20351_Old_Television_set_1980s_style_v2.obj");
	//Model mujer((char*)"Models/091_W_Aya_100K.obj");
	Model escaleras((char*)"Models/stairs/escaleras_eriq.obj");
	Model escritorio((char*)"Models/Desk/desk.obj");
	Model lampara((char*)"Models/Lampara/lampara.obj");

	//Model BotaDer((char*)"Models/Personaje/bota.obj");
	//Model PiernaDer((char*)"Models/Personaje/piernader.obj");
	//Model PiernaIzq((char*)"Models/Personaje/piernaizq.obj");
	//Model Torso((char*)"Models/Personaje/torso.obj");
	//Model BrazoDer((char*)"Models/Personaje/brazoder.obj");
	//Model BrazoIzq((char*)"Models/Personaje/brazoizq.obj");
	//Model Cabeza((char*)"Models/Personaje/cabeza.obj");

	//Carga de texturas primitivas (esto iba antes de glEnable)
	LoadTextures();

	// Build and compile our shader program


	//Inicialización de KeyFrames

	for (int i = 0; i < MAX_FRAMES; i++)
	{
		KeyFrame[i].posX = 0;
		KeyFrame[i].incX = 0;
		KeyFrame[i].incY = 0;
		KeyFrame[i].incZ = 0;
		KeyFrame[i].rotRodIzq = 0;
		KeyFrame[i].rotRodDer = 0;
		KeyFrame[i].rotInc = 0;
		KeyFrame[i].rotInc2 = 0;

		KeyFrame[i].rotBraIzq = 0;
		KeyFrame[i].rotBraDer = 0;
		KeyFrame[i].rotInc3 = 0;
		KeyFrame[i].rotInc4 = 0;
	}



	// Set up vertex data (and buffer(s)) and attribute pointers
	GLfloat vertices[] =
	{
		// Positions            // Normals              // Texture Coords
		//Plano superior izquierdo
		+0.0f, +0.0f, 0.0f,    0.0f, 0.0f, 1.0f,		0.0f,0.0f,
		+1.0f, +0.0f, 0.0f,	   0.0f, 0.0f, 1.0f,		1.0f,0.0f,
		+1.0f, +1.0f, 0.0f,    0.0f, 0.0f, 1.0f,		1.0f,1.0f,
		+0.0f, +1.0f, 0.0f,    0.0f, 0.0f, 1.0f,		0.0f,1.0f,

		////Para dibujar utilizando glDrawArrays y modificando coordenadas de texturizado para piso de la puerta

		+0.0f, +0.0f, 0.0f,    0.0f, 0.0f, -1.0f,		0.75f,0.75f, //A
		+1.0f, +0.0f, 0.0f,	   0.0f, 0.0f, -1.0f,		1.0f,0.75f, //B
		+0.0f, +1.0f, 0.0f,    0.0f, 0.0f, -1.0f,		0.75f,1.0f, //C
		+1.0f, +0.0f, 0.0f,	   0.0f, 0.0f, -1.0f,		1.0f,0.75f, //D
		+1.0f, +1.0f, 0.0f,    0.0f, 0.0f, -1.0f,		1.0f,1.0f, //E
		+0.0f, +1.0f, 0.0f,    0.0f, 0.0f, -1.0f,		0.75f, 1.0f, //F



	};

	GLfloat skyboxVertices[] = {
		// Positions
		-1.0f,  1.0f, -1.0f,
		-1.0f, -1.0f, -1.0f,
		1.0f, -1.0f, -1.0f,
		1.0f, -1.0f, -1.0f,
		1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,

		-1.0f, -1.0f,  1.0f,
		-1.0f, -1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f,  1.0f,
		-1.0f, -1.0f,  1.0f,

		1.0f, -1.0f, -1.0f,
		1.0f, -1.0f,  1.0f,
		1.0f,  1.0f,  1.0f,
		1.0f,  1.0f,  1.0f,
		1.0f,  1.0f, -1.0f,
		1.0f, -1.0f, -1.0f,

		-1.0f, -1.0f,  1.0f,
		-1.0f,  1.0f,  1.0f,
		1.0f,  1.0f,  1.0f,
		1.0f,  1.0f,  1.0f,
		1.0f, -1.0f,  1.0f,
		-1.0f, -1.0f,  1.0f,

		-1.0f,  1.0f, -1.0f,
		1.0f,  1.0f, -1.0f,
		1.0f,  1.0f,  1.0f,
		1.0f,  1.0f,  1.0f,
		-1.0f,  1.0f,  1.0f,
		-1.0f,  1.0f, -1.0f,

		-1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f,  1.0f,
		1.0f, -1.0f, -1.0f,
		1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f,  1.0f,
		1.0f, -1.0f,  1.0f
	};

	GLuint indices[] =
	{  // Note that we start from 0!
		0,1,3,
		1,2,3,

		0,1,2,3,
		4,5,6,7,
		8,9,10,11,
		12,13,14,15,
		16,17,18,19,
		20,21,22,23,
		24,25,26,27,
		28,29,30,31,
		32,33,34,35

	};

	// Positions all containers
	glm::vec3 cubePositions[] = {
		glm::vec3(0.0f,  0.0f,  0.0f)
	};


	// First, set the container's VAO (and VBO)
	GLuint VBO, VAO, EBO;
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);

	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	// Position attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid *)0);
	glEnableVertexAttribArray(0);
	// Normals attribute
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid *)(3 * sizeof(GLfloat)));
	glEnableVertexAttribArray(1);
	// Texture Coordinate attribute
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid *)(6 * sizeof(GLfloat)));
	glEnableVertexAttribArray(2);
	glBindVertexArray(0);

	// Then, we set the light's VAO (VBO stays the same. After all, the vertices are the same for the light object (also a 3D cube))
	GLuint lightVAO;
	glGenVertexArrays(1, &lightVAO);
	glBindVertexArray(lightVAO);
	// We only need to bind to the VBO (to link it with glVertexAttribPointer), no need to fill it; the VBO's data already contains all we need.
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	// Set the vertex attributes (only position data for the lamp))
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid *)0); // Note that we skip over the other data in our buffer object (we don't need the normals/textures, only positions).
	glEnableVertexAttribArray(0);
	glBindVertexArray(0);

	//SkyBox
	GLuint skyboxVBO, skyboxVAO;
	glGenVertexArrays(1, &skyboxVAO);
	glGenBuffers(1, &skyboxVBO);
	glBindVertexArray(skyboxVAO);
	glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);

	// Load textures
	vector<const GLchar*> faces;
	faces.push_back("SkyBox/right.tga");
	faces.push_back("SkyBox/left.tga");
	faces.push_back("SkyBox/top.tga");
	faces.push_back("SkyBox/bottom.tga");
	faces.push_back("SkyBox/back.tga");
	faces.push_back("SkyBox/front.tga");

	GLuint cubemapTexture = TextureLoading::LoadCubemap(faces);


	//glBindTexture(GL_TEXTURE_2D, 0);

	// Set texture units
	lightingShader.Use();
	glUniform1i(glGetUniformLocation(lightingShader.Program, "material.diffuse"), 0);
	glUniform1i(glGetUniformLocation(lightingShader.Program, "material.specular"), 1);

	glm::mat4 projection = glm::perspective(camera.GetZoom(), (GLfloat)SCREEN_WIDTH / (GLfloat)SCREEN_HEIGHT, 0.1f, 100.0f);
	//glm::mat4 projection = glm::ortho(-5.0f, 5.0f, -3.0f, 3.0f, 0.1f, 10.0f);

	// Game loop
	while (!glfwWindowShouldClose(window))
	{

		//Mostrar va

		// Calculate deltatime of current frame
		GLfloat currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		// Check if any events have been activiated (key pressed, mouse moved etc.) and call corresponding response functions
		glfwPollEvents();
		DoMovement();
		//animacion();

		// Clear the colorbuffer
		glClearColor(0.9f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Use cooresponding shader when setting uniforms/drawing objects
		lightingShader.Use();
		GLint viewPosLoc = glGetUniformLocation(lightingShader.Program, "viewPos");
		glUniform3f(viewPosLoc, camera.GetPosition().x, camera.GetPosition().y, camera.GetPosition().z);
		// Set material properties
		glUniform1f(glGetUniformLocation(lightingShader.Program, "material.shininess"), 4.0f);
		// == ==========================
		// Here we set all the uniforms for the 5/6 types of lights we have. We have to set them manually and index
		// the proper PointLight struct in the array to set each uniform variable. This can be done more code-friendly
		// by defining light types as classes and set their values in there, or by using a more efficient uniform approach
		// by using 'Uniform buffer objects', but that is something we discuss in the 'Advanced GLSL' tutorial.
		// == ==========================
		// Luz direccional (Sol)
		glUniform3f(glGetUniformLocation(lightingShader.Program, "dirLight.direction"), 0.0f, 0.0f, 0.0f);
		glUniform3f(glGetUniformLocation(lightingShader.Program, "dirLight.ambient"), 0.05f, 0.05f, 0.05f);
		glUniform3f(glGetUniformLocation(lightingShader.Program, "dirLight.diffuse"), 0.2f, 0.2f, 0.2f);
		glUniform3f(glGetUniformLocation(lightingShader.Program, "dirLight.specular"), 0.2f, 0.2f, 0.2f);
			
		// Luz puntual (Foco)
		//glUniform3f(glGetUniformLocation(lightingShader.Program, "pointLights[0].position"), pointLightPositions[0].x, pointLightPositions[0].y, pointLightPositions[0].z);
		glUniform3f(glGetUniformLocation(lightingShader.Program, "pointLights[0].position"), 4.67f, 1.2f, 3.5f);
		glUniform3f(glGetUniformLocation(lightingShader.Program, "pointLights[0].ambient"), 0.0f, 0.0f, 0.0f);
		glUniform3f(glGetUniformLocation(lightingShader.Program, "pointLights[0].diffuse"), 0.9f, 0.9f, 0.9f);
		glUniform3f(glGetUniformLocation(lightingShader.Program, "pointLights[0].specular"), 0.0f, 0.0f, 0.0f);
		glUniform1f(glGetUniformLocation(lightingShader.Program, "pointLights[0].constant"), 1.0f);
		glUniform1f(glGetUniformLocation(lightingShader.Program, "pointLights[0].linear"), 0.14);
		glUniform1f(glGetUniformLocation(lightingShader.Program, "pointLights[0].quadratic"), 0.07);

		// Luz puntual 1 (Foco)
		glUniform3f(glGetUniformLocation(lightingShader.Program, "pointLights[1].position"), 0.1+0.13, 0.80+0.10, 4.90);
		glUniform3f(glGetUniformLocation(lightingShader.Program, "pointLights[1].ambient"), 0.0f, 0.0f, 0.0f);
		glUniform3f(glGetUniformLocation(lightingShader.Program, "pointLights[1].diffuse"), 0.99f, 0.99f, 0.99f);
		glUniform3f(glGetUniformLocation(lightingShader.Program, "pointLights[1].specular"), 0.1f, 0.1f, 0.1f);
		glUniform1f(glGetUniformLocation(lightingShader.Program, "pointLights[1].constant"), 1.0f);
		glUniform1f(glGetUniformLocation(lightingShader.Program, "pointLights[1].linear"), 0.14);
		glUniform1f(glGetUniformLocation(lightingShader.Program, "pointLights[1].quadratic"), 0.07);

		// Luz puntual 2 (Foco)
		glUniform3f(glGetUniformLocation(lightingShader.Program, "pointLights[2].position"), pointLightPositions[2].x, pointLightPositions[2].y, pointLightPositions[2].z);
		glUniform3f(glGetUniformLocation(lightingShader.Program, "pointLights[2].ambient"), 0.0f, 0.0f, 0.0f);
		glUniform3f(glGetUniformLocation(lightingShader.Program, "pointLights[2].diffuse"), 0.6f, 0.6f, 0.6f);
		glUniform3f(glGetUniformLocation(lightingShader.Program, "pointLights[2].specular"), 0.4f, 0.4f, 0.4f);
		glUniform1f(glGetUniformLocation(lightingShader.Program, "pointLights[2].constant"), 1.0f);
		glUniform1f(glGetUniformLocation(lightingShader.Program, "pointLights[2].linear"), 0.14);
		glUniform1f(glGetUniformLocation(lightingShader.Program, "pointLights[2].quadratic"), 0.07);


		// SpotLight (Lámpara de mano)
		glUniform3f(glGetUniformLocation(lightingShader.Program, "spotLight.position"), 1.55f, 0.58f+0.48, 0.0f); 
		glUniform3f(glGetUniformLocation(lightingShader.Program, "spotLight.direction"), 0.0f, -1.0f-5.18f, 1.97f+4.68); //Vector forward de la camara
		glUniform3f(glGetUniformLocation(lightingShader.Program, "spotLight.ambient"), 0, 0, 0);
		glUniform3f(glGetUniformLocation(lightingShader.Program, "spotLight.diffuse"), LightP1.x * 0.984, LightP1.y * 0.905, LightP1.z * 0.376); //Anaranjado
		glUniform3f(glGetUniformLocation(lightingShader.Program, "spotLight.specular"), LightP1.x * 0.3f, LightP1.y * 0.3f, LightP1.z * 0.3f); //x2
		glUniform1f(glGetUniformLocation(lightingShader.Program, "spotLight.constant"), 0.6f); //intensidad
		glUniform1f(glGetUniformLocation(lightingShader.Program, "spotLight.linear"), 0.05f);
		glUniform1f(glGetUniformLocation(lightingShader.Program, "spotLight.quadratic"), 0.032f);
		glUniform1f(glGetUniformLocation(lightingShader.Program, "spotLight.cutOff"), glm::cos(glm::radians(15.0f))); //Cono interior
		glUniform1f(glGetUniformLocation(lightingShader.Program, "spotLight.outerCutOff"), glm::cos(glm::radians(23.0f+20.25+10))); //Cono exterior

		// Setear propiedades del material
		glUniform1f(glGetUniformLocation(lightingShader.Program, "material.shininess"), 32.0f);

		// Crear transformaciones de cámara
		glm::mat4 view;
		view = camera.GetViewMatrix();

		// Get the uniform locations
		GLint modelLoc = glGetUniformLocation(lightingShader.Program, "model");
		GLint viewLoc = glGetUniformLocation(lightingShader.Program, "view");
		GLint projLoc = glGetUniformLocation(lightingShader.Program, "projection");

		// Pasar matrices al Shader
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

		// Enlazar mapa difuso
		//glActiveTexture(GL_TEXTURE0);
		//glBindTexture(GL_TEXTURE_2D, specular_map);

		// Enlazar mapa especular
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, specular_map);

		glBindVertexArray(VAO);

		//*****COMIENZA EL DIBUJADO DE LA GEOMETRÍA*****
		
		glm::mat4 model(1);
		glm::mat4 temp(1);

		// Carga mapa difuso (uso de texturas normales)
		glActiveTexture(GL_TEXTURE0);
        
		//Suelo de madera
		glBindTexture(GL_TEXTURE_2D, texture0);
        model = glm::scale(model, glm::vec3(4.0f, 4.0f, 6.0f)); //Escala primero
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 1.0f)); //Mueve hacia Z+
        model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f)); //Acuesta la geometría, quedando la normal hacia Y
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        //glBindVertexArray(0);


		//Tapete
		//El tapete no refleja especular
		glUniform3f(glGetUniformLocation(lightingShader.Program, "pointLights[0].specular"), 0.0f, 0.0f, 0.0f);
        glBindTexture(GL_TEXTURE_2D, texture1); 
        model = glm::mat4(1);
        model = glm::scale(model, glm::vec3(2.35f, 2.35f, 3.5f));
		model = glm::translate(model, glm::vec3(0.3f, 0.0001, 1.40f));
        model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		model = glm::rotate(model, glm::radians(5.0f), glm::vec3(0.0f, 0.0f, 1.0f)); //Rota ligeramente el tapete
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

		//Los muros reflejan especular a medias
		glUniform3f(glGetUniformLocation(lightingShader.Program, "pointLights[0].specular"), 0.35f, 0.35f, 0.35f);

		//Muros frontales
		model = glm::mat4(1);
		glBindTexture(GL_TEXTURE_2D, texture2);
		model = glm::scale(model, glm::vec3(1.0f, 1.5f, 1.0f)); //Escala primero
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

		model = glm::translate(model, glm::vec3(1.0f, 0.0f, 0.0f));
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

		model = glm::translate(model, glm::vec3(1.0f, 0.0f, 0.0f));
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

		//Muro izquierdo de la puerta
		model = glm::translate(model, glm::vec3(1.0f, 0.0f, 0.0f));
		model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);


		//Muro puerta (poner una pierta aqui)
		model = glm::translate(model, glm::vec3(1.0f, 0.0f, 0.0f));
		temp = model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
		//glDrawArrays(GL_TRIANGLES, 4, 9);

		//Puerta sin brillo
		glUniform3f(glGetUniformLocation(lightingShader.Program, "pointLights[0].specular"), 0.2f, 0.2f, 0.2f);
		glUniform1f(glGetUniformLocation(lightingShader.Program, "material.shininess"), 8.0f);

		//Puerta blanca
		glBindTexture(GL_TEXTURE_2D, texture8);
		model = glm::translate(model, glm::vec3(0.12f, 0.0f, 0.001f));
		model = glm::scale(model, glm::vec3(0.7f, 0.8f, 1.0f));
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

		//Regresa la luz a la normalidad
		glUniform3f(glGetUniformLocation(lightingShader.Program, "pointLights[0].specular"), 0.5f, 0.5f, 0.5f);
		glUniform1f(glGetUniformLocation(lightingShader.Program, "material.shininess"), 32.0f);

		//glBindTexture(GL_TEXTURE_2D, texture2); //Carga textura muro de nuevo

		//Suelo de madera frente a la puerta
		//Realiza cambio de textura a madera
		glBindTexture(GL_TEXTURE_2D, texture0);
		model = glm::rotate(temp, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f)); //Gira X para acostar el plano (cambio normal en vértices 4 a 9)
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		glDrawArrays(GL_TRIANGLES, 4, 9);

		//Muro derecho de la puerta
		//Carga de nuevo textura de papel tapiz y dibuja muro derecho
		glBindTexture(GL_TEXTURE_2D, texture2);
		model = glm::translate(model, glm::vec3(1.0f, 0.0f, 0.0f)); //Desplaza hacia la derecha el plano
		model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f)); //Levántalo
		model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f)); //Rota en Y 90° para quedar a la derecha
		
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);


		//Muros izquierdos
		model = glm::mat4(1);
		glBindTexture(GL_TEXTURE_2D, texture2);
		model = glm::scale(model, glm::vec3(1.0f, 1.5f, 1.0f)); //Escala primero
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 1.0f));
		temp = model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

		//Dibujar cuadro2 sobre muro 1
		//Matriz temporal, almacena muro anterior sin afectar por el cuadro
		
		//Cuadro muy brillante
		glUniform3f(glGetUniformLocation(lightingShader.Program, "pointLights[0].specular"), 0.9f, 0.9f, 0.9f);
		glUniform1f(glGetUniformLocation(lightingShader.Program, "material.shininess"), 96.0f);

		glBindTexture(GL_TEXTURE_2D, texture5);
		model = glm::translate(model, glm::vec3(-0.28f, 0.48, 0.001f));
		model = glm::scale(model, glm::vec3(0.4f, 0.30f, 0.30f));
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

		//Regresa la luz a la normalidad
		glUniform3f(glGetUniformLocation(lightingShader.Program, "pointLights[0].specular"), 0.5f, 0.5f, 0.5f);
		glUniform1f(glGetUniformLocation(lightingShader.Program, "material.shininess"), 32.0f);
		
		//Muro 1, recupera contexto del muro 1
		glBindTexture(GL_TEXTURE_2D, texture2); //Carga textura muro de nuevo
		temp = model = glm::translate(temp, glm::vec3(-1.0f, 0.0f, 0.0f));
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);


		//Dibujar cuadro1 sobre muro 2
		//Matriz temporal, almacena muro anterior sin afectar por el cuadro
		
		//Cuadro muy brillante
		glUniform3f(glGetUniformLocation(lightingShader.Program, "pointLights[0].specular"), 0.9f, 0.9f, 0.9f);
		glUniform1f(glGetUniformLocation(lightingShader.Program, "material.shininess"), 96.0f);

		glBindTexture(GL_TEXTURE_2D, texture4);
		model = glm::translate(model, glm::vec3(-0.45f, 0.43f, 0.001f));
		model = glm::scale(model, glm::vec3(0.5f, 0.35f, 0.35f)); 
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);


		//Regresa la luz a la normalidad
		glUniform3f(glGetUniformLocation(lightingShader.Program, "pointLights[0].specular"), 0.5f, 0.5f, 0.5f);
		glUniform1f(glGetUniformLocation(lightingShader.Program, "material.shininess"), 32.0f);

		//Muro2, recupera contexto del muro 1
		glBindTexture(GL_TEXTURE_2D, texture2); //Carga textura muro de nuevo
		temp = model = glm::translate(temp, glm::vec3(-1.0f, 0.0f, 0.0f));
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

		//Puerta sin brillo
		glUniform3f(glGetUniformLocation(lightingShader.Program, "pointLights[0].specular"), 0.3f, 0.3f, 0.3f);
		glUniform1f(glGetUniformLocation(lightingShader.Program, "material.shininess"), 8.0f);

		//Puerta café de madera
		glBindTexture(GL_TEXTURE_2D, texture9);
		model = glm::translate(model, glm::vec3(0.10f-0.82, 0.0f, 0.001f));
		model = glm::scale(model, glm::vec3(0.7f, 0.8f, 1.0f));
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);


		//Regresa la luz a la normalidad
		glUniform3f(glGetUniformLocation(lightingShader.Program, "pointLights[0].specular"), 0.5f, 0.5f, 0.5f);
		glUniform1f(glGetUniformLocation(lightingShader.Program, "material.shininess"), 32.0f);

		glBindTexture(GL_TEXTURE_2D, texture2);
		model = glm::translate(temp, glm::vec3(-1.0f, 0.0f, 0.0f));
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

		model = glm::translate(model, glm::vec3(-1.0f, 0.0f, 0.0f));
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

		model = glm::translate(model, glm::vec3(-1.0f, 0.0f, 0.0f));
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

		//Muros derechos

		model = glm::mat4(1);
		glBindTexture(GL_TEXTURE_2D, texture2);
		model = glm::scale(model, glm::vec3(1.0f, 1.5f, 1.0f)); //Escala primero
		model = glm::translate(model, glm::vec3(4.0f, 0.0f, 0.0f));
		model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

		model = glm::translate(model, glm::vec3(1.0f, 0.0f, 0.0f));
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

		temp = model = glm::translate(model, glm::vec3(1.0f, 0.0f, 0.0f));
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);


		//Cuadro muy brillante
		glUniform3f(glGetUniformLocation(lightingShader.Program, "pointLights[0].specular"), 0.9f, 0.9f, 0.9f);
		glUniform1f(glGetUniformLocation(lightingShader.Program, "material.shininess"), 96.0f);

		//Cuadro mamá de CJ
		glBindTexture(GL_TEXTURE_2D, texture7);
		model = glm::translate(model, glm::vec3(-0.28f, 0.48, 0.001f));
		model = glm::scale(model, glm::vec3(0.4f, 0.30f, 0.30f));
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

		//Regresa la luz a la normalidad
		glUniform3f(glGetUniformLocation(lightingShader.Program, "pointLights[0].specular"), 0.5f, 0.5f, 0.5f);
		glUniform1f(glGetUniformLocation(lightingShader.Program, "material.shininess"), 32.0f);

		//Baja la especular para el foco de las escaleras

		glUniform3f(glGetUniformLocation(lightingShader.Program, "pointLights[0].specular"), 0.1f, 0.1f, 0.1f);

		glBindTexture(GL_TEXTURE_2D, texture2); //Carga textura muro de nuevo
		//Gira el muro para dibujar escalera
		model = glm::translate(temp, glm::vec3(1.0f, 0.0f, 0.0f));
		model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);


		model = glm::translate(model, glm::vec3(1.0f, 0.0f, 0.0f));
		model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

		//Rota para la escalera
		model = glm::translate(model, glm::vec3(1.0f, 0.0f, 0.0f));
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

		//Recupera el brillo especular original

		glUniform3f(glGetUniformLocation(lightingShader.Program, "pointLights[0].specular"), 0.5f, 0.5f, 0.5f);

		model = glm::translate(model, glm::vec3(1.0f, 0.0f, 0.0f));
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);


		model = glm::translate(model, glm::vec3(1.0f, 0.0f, 0.0f));
		model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

		model = glm::translate(model, glm::vec3(1.0f, 0.0f, 0.0f));
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

		model = glm::translate(model, glm::vec3(1.0f, 0.0f, 0.0f));
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);


		model = glm::translate(model, glm::vec3(1.0f, 0.0f, 0.0f));
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

		model = glm::translate(model, glm::vec3(1.0f, 0.0f, 0.0f));
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);


		//Marco de la ventana
		glUniform3f(glGetUniformLocation(lightingShader.Program, "pointLights[0].specular"), 1.0f, 1.0f, 1.0f);

		model = glm::mat4(1);
		model = glm::translate(model, glm::vec3(0.92f, 0.48f, 0.001f));
		model = glm::scale(model, glm::vec3(1.25f, 0.65f, 1.0f));
		glBindTexture(GL_TEXTURE_2D, texture3);
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

		glUniform3f(glGetUniformLocation(lightingShader.Program, "pointLights[0].specular"), 0.5f, 0.5f, 0.5f);



		//****Carga de modelos OBJ****

		//Sillón doble
		model = glm::mat4(1);
		model = glm::translate(model, glm::vec3(-2.10 + 4.0f, 0.0f, 3.4f)); // Translate it down a bit so it's at the center of the scene
		model = glm::scale(model, glm::vec3(0.20f, 0.20f, 0.20f));	// It's a bit too big for our scene, so scale it down
		model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		glUniformMatrix4fv(glGetUniformLocation(lightingShader.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));

		sillon.Draw(lightingShader);

		//Mesa central sala
		model = glm::mat4(1);
		model = glm::translate(model, glm::vec3(-2.4 + 4.0f, 0.0f, 2.10f)); // Translate it down a bit so it's at the center of the scene
		model = glm::scale(model, glm::vec3(0.2f, 0.2f, 0.2f));	// It's a bit too big for our scene, so scale it down
		model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		glUniformMatrix4fv(glGetUniformLocation(lightingShader.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));

		mesa_centro.Draw(lightingShader);


		//shader.Use();
		//Planta esa central sala
		model = glm::mat4(1);
		model = glm::translate(model, glm::vec3(-2.4 + 4.0f, 0.225f, 2.10f)); // Translate it down a bit so it's at the center of the scene
		model = glm::scale(model, glm::vec3(0.10f, 0.10f, 0.10f));	// It's a bit too big for our scene, so scale it down
		model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		glUniformMatrix4fv(glGetUniformLocation(lightingShader.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));

		planta_mesa_centro.Draw(lightingShader);

		//Sofa individual 1
		model = glm::mat4(1);
		model = glm::translate(model, glm::vec3(-1.4+ 4.0f, 0.0f, 2.5f)); // Translate it down a bit so it's at the center of the scene
		model = glm::scale(model, glm::vec3(0.10f, 0.1f, 0.1f));	// It's a bit too big for our scene, so scale it down
		model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		glUniformMatrix4fv(glGetUniformLocation(lightingShader.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));


		sofa.Draw(lightingShader);

		//Sofa individual 2
		//model = glm::mat4(1);
		model = glm::translate(model, glm::vec3(-10.0 + 4.0f, 0.0f, 0.0f)); // Translate it down a bit so it's at the center of the scene
		//model = glm::scale(model, glm::vec3(0.15f, 0.15f, 0.15f));	// It's a bit too big for our scene, so scale it down
		model = glm::rotate(model, glm::radians(30.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		glUniformMatrix4fv(glGetUniformLocation(lightingShader.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));

		sofa.Draw(lightingShader);

		model = glm::mat4(1);
		model = glm::translate(model, glm::vec3(1.55f, 0.0f, 0.08f)); // Translate it down a bit so it's at the center of the scene
		model = glm::scale(model, glm::vec3(0.17f, 0.17f, 0.17));	// It's a bit too big for our scene, so scale it down
		//model = glm::rotate(model, glm::radians(-60.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		glUniformMatrix4fv(glGetUniformLocation(lightingShader.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
		
		cortina.Draw(lightingShader);


		//Mesa tv
		model = glm::mat4(1);
		model = glm::translate(model, glm::vec3(0.30f, 0.0f, 0.30)); // Translate it down a bit so it's at the center of the scene
		//model = glm::scale(model, glm::vec3(0.2f, 0.2f, 0.2f));	// It's a bit too big for our scene, so scale it down
		model = glm::rotate(model, glm::radians(45.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::scale(model, glm::vec3(0.1f, 0.2f, 0.2f));	// It's a bit too big for our scene, so scale it down
		glUniformMatrix4fv(glGetUniformLocation(lightingShader.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));

		mesa_centro.Draw(lightingShader);


		model = glm::mat4(1);
		model = glm::translate(model, glm::vec3(0.40f, 0.20f, 0.4f)); // Translate it down a bit so it's at the center of the scene
		model = glm::scale(model, glm::vec3(0.08f, 0.08f, 0.08f));	// It's a bit too big for our scene, so scale it down
		model = glm::rotate(model, glm::radians(45.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		glUniformMatrix4fv(glGetUniformLocation(lightingShader.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));

		tv.Draw(lightingShader);


		////Regresa la luz a la normalidad
		//glUniform3f(glGetUniformLocation(lightingShader.Program, "pointLights[0].specular"), 0.1f, 0.1f, 0.1f);
		//glUniform1f(glGetUniformLocation(lightingShader.Program, "material.shininess"), 16.0f);

		model = glm::mat4(1);
		model = glm::translate(model, glm::vec3(3.64, 0.0f, 3.45+0.12)); // Translate it down a bit so it's at the center of the scene
		model = glm::scale(model, glm::vec3(0.12, 0.1f, 0.08+0.02));	// It's a bit too big for our scene, so scale it down
		model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		glUniformMatrix4fv(glGetUniformLocation(lightingShader.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));

		escaleras.Draw(lightingShader);

		model = glm::mat4(1);
		model = glm::translate(model, glm::vec3(3.7f, 0.0f, 1.6)); // Translate it down a bit so it's at the center of the scene
		model = glm::scale(model, glm::vec3(0.4f));	// It's a bit too big for our scene, so scale it down
		model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		glUniformMatrix4fv(glGetUniformLocation(lightingShader.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));

		escritorio.Draw(lightingShader);

		//glUniform3f(glGetUniformLocation(lightingShader.Program, "pointLights[0].specular"), 0.2f, 0.2f, 0.2f);
		//glUniform3f(glGetUniformLocation(lightingShader.Program, "pointLights[1].specular"), 0.2f, 0.2f, 0.2f);
		//glUniform1f(glGetUniformLocation(lightingShader.Program, "material.shininess"), 2.0f);

		//model = glm::mat4(1);
		//model = glm::translate(model, glm::vec3(movX, movY, movZ)); // Translate it down a bit so it's at the center of the scene
		//model = glm::scale(model, glm::vec3(0.3f, 0.3f, 0.3f));	// It's a bit too big for our scene, so scale it down
		//model = glm::rotate(model, glm::radians(-60.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		//glUniformMatrix4fv(glGetUniformLocation(lightingShader.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));

		//mujer.Draw(lightingShader);


		printf("X= %0.2f Y= %0.2f, Z= %0.2f\n", movX, movY, movZ);



		glBindVertexArray(0);

		// Also draw the lamp object, again binding the appropriate shader
		lampShader.Use();
		// Get location objects for the matrices on the lamp shader (these could be different on a different shader)
		modelLoc = glGetUniformLocation(lampShader.Program, "model");
		viewLoc = glGetUniformLocation(lampShader.Program, "view");
		projLoc = glGetUniformLocation(lampShader.Program, "projection");

		// Set matrices
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));
		model = glm::mat4(1);
		model = glm::translate(model, lightPos);
		model = glm::scale(model, glm::vec3(0.2f)); // Make it a smaller cube
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		// Draw the light object (using light's vertex attributes)
		//glBindVertexArray(lightVAO);
		glBindVertexArray(VAO);

		//Dibuja las luces puntuales
		for (GLuint i = 1; i < 2; i++)
		{
			model = glm::mat4(1);
			model = glm::translate(model, pointLightPositions[i]);
			model = glm::scale(model, glm::vec3(0.05f)); // Make it a smaller cube
			glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
			//glDrawArrays(GL_TRIANGLES, 0, 3);
			glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);


			printf("%f %f %f\n", pointLightPositions[1].x, pointLightPositions[1].y, pointLightPositions[1].z);
		}

		glBindVertexArray(0); //Fin uso lampshader


		//*************SHADER MODELOS SIN ILUMINACIÓN**************
		modelLoading.Use();
		modelLoc = glGetUniformLocation(lampShader.Program, "model");
		viewLoc = glGetUniformLocation(lampShader.Program, "view");
		projLoc = glGetUniformLocation(lampShader.Program, "projection");

		// Set matrices
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

		model = glm::mat4(1);
		//model = glm::translate(model, lightPos);
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		glBindVertexArray(VAO); //Debería ser LightVAO

		//Vidrios de la ventana
		model = glm::mat4(1);
		model = glm::translate(model, glm::vec3(0.92f, 0.48f, 0.001f));
		model = glm::scale(model, glm::vec3(1.25f, 0.65f, 1.0f));
		glBindTexture(GL_TEXTURE_2D, texture6);
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

		model = glm::mat4(1);
		model = glm::translate(model, glm::vec3(5.0f, 1.05f, 3.5f)); // Translate it down a bit so it's at the center of the scene
		model = glm::scale(model, glm::vec3(0.3f));	// It's a bit too big for our scene, so scale it down
		model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		glUniformMatrix4fv(glGetUniformLocation(lightingShader.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));

		lampara.Draw(modelLoading);

		model = glm::mat4(1);
		model = glm::translate(model, glm::vec3(0.03, 0.85, 4.90f)); // Translate it down a bit so it's at the center of the scene
		model = glm::scale(model, glm::vec3(0.3f));	// It's a bit too big for our scene, so scale it down
		model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		glUniformMatrix4fv(glGetUniformLocation(lightingShader.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));

		lampara.Draw(modelLoading);

		glBindVertexArray(0);

		// Draw skybox as last
		glDepthFunc(GL_LEQUAL);  // Change depth function so depth test passes when values are equal to depth buffer's content
		SkyBoxshader.Use();
		view = glm::mat4(glm::mat3(camera.GetViewMatrix()));	// Remove any translation component of the view matrix
		glUniformMatrix4fv(glGetUniformLocation(SkyBoxshader.Program, "view"), 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(glGetUniformLocation(SkyBoxshader.Program, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

		// skybox cube
		glBindVertexArray(skyboxVAO);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
		glDrawArrays(GL_TRIANGLES, 6, 42);
		glBindVertexArray(0);
		glDepthFunc(GL_LESS); // Set depth function back to default


		// Swap the screen buffers
		glfwSwapBuffers(window);
	}



	glDeleteVertexArrays(1, &VAO);
	glDeleteVertexArrays(1, &lightVAO);
	glDeleteBuffers(1, &VBO);
	glDeleteBuffers(1, &EBO);
	glDeleteVertexArrays(1, &skyboxVAO);
	glDeleteBuffers(1, &skyboxVBO);

	// Terminate GLFW, clearing any resources allocated by GLFW.
	glfwTerminate();



	return 0;
}

void animacion()
{

	//Movimiento del personaje

	if (play)
	{
		if (i_curr_steps >= i_max_steps) //end of animation between frames?
		{
			playIndex++;
			if (playIndex > FrameIndex - 2)	//end of total animation?
			{
				printf("termina anim\n");
				playIndex = 0;
				play = false;
			}
			else //Next frame interpolations
			{
				i_curr_steps = 0; //Reset counter
								  //Interpolation
				interpolation();
			}
		}
		else
		{
			//Draw animation
			posX += KeyFrame[playIndex].incX;
			posY += KeyFrame[playIndex].incY;
			posZ += KeyFrame[playIndex].incZ;

			rotRodIzq += KeyFrame[playIndex].rotInc;
			rotRodDer += KeyFrame[playIndex].rotInc2;

			rotBraIzq += KeyFrame[playIndex].rotInc3;
			rotBraDer += KeyFrame[playIndex].rotInc4;

			i_curr_steps++;
		}

	}
}

// Moves/alters the camera positions based on user input
void DoMovement()
{
	if (keys[GLFW_KEY_1])
	{
		rot += 1;
	}

	if (keys[GLFW_KEY_2])
	{
		if (rotRodIzq < 80.0f)
			rotRodIzq += 1.0f;

	}

	if (keys[GLFW_KEY_3])
	{
		if (rotRodIzq > -45)
			rotRodIzq -= 1.0f;

	}

	if (keys[GLFW_KEY_4])
	{
		if (rotRodDer < 80.0f)
			rotRodDer += 1.0f;

	}

	if (keys[GLFW_KEY_5])
	{
		if (rotRodDer > -45)
			rotRodDer -= 1.0f;

	}

	if (keys[GLFW_KEY_6])
	{
		if (rotBraDer < 80.0f)
			rotBraDer += 1.0f;

	}

	if (keys[GLFW_KEY_7])
	{
		if (rotBraDer > -45)
			rotBraDer -= 1.0f;

	}



	//Mov Personaje
	if (keys[GLFW_KEY_H])
	{
		posZ += 1;
	}

	if (keys[GLFW_KEY_Y])
	{
		posZ -= 1;
	}

	if (keys[GLFW_KEY_G])
	{
		posX -= 1;
	}

	if (keys[GLFW_KEY_J])
	{
		posX += 1;
	}


	// Camera controls
	if (keys[GLFW_KEY_W] || keys[GLFW_KEY_UP])
	{
		camera.ProcessKeyboard(FORWARD, deltaTime);

	}

	if (keys[GLFW_KEY_S] || keys[GLFW_KEY_DOWN])
	{
		camera.ProcessKeyboard(BACKWARD, deltaTime);


	}

	if (keys[GLFW_KEY_A] || keys[GLFW_KEY_LEFT])
	{
		camera.ProcessKeyboard(LEFT, deltaTime);


	}

	if (keys[GLFW_KEY_D] || keys[GLFW_KEY_RIGHT])
	{
		camera.ProcessKeyboard(RIGHT, deltaTime);
	}

	if (keys[GLFW_KEY_F])
	{
		pointLightPositions[2].x -= 0.1f;
	}
	if (keys[GLFW_KEY_H])
	{
		pointLightPositions[2].x += 0.1f;
	}

	if (keys[GLFW_KEY_T])
	{
		pointLightPositions[2].y += 0.1f;
	}

	if (keys[GLFW_KEY_G])
	{
		pointLightPositions[2].y -= 0.1f;
	}
	if (keys[GLFW_KEY_R])
	{
		pointLightPositions[2].z -= 0.1f;
	}
	if (keys[GLFW_KEY_Y])
	{
		pointLightPositions[2].z += 0.1f;
	}

	//Moviendo con teclas el objeto
	if (keys[GLFW_KEY_J])
	{
		movX -= 0.025f;
	}
	if (keys[GLFW_KEY_L])
	{
		movX += 0.025f;
	}
	if (keys[GLFW_KEY_I])
	{
		movY += 0.025f;
	}
	if (keys[GLFW_KEY_K])
	{
		movY -= 0.025f;
	}
	if (keys[GLFW_KEY_U])
	{
		movZ -= 0.025f;
	}
	if (keys[GLFW_KEY_O])
	{
		movZ += 0.025f;
	}
}

// Is called whenever a key is pressed/released via GLFW
void KeyCallback(GLFWwindow *window, int key, int scancode, int action, int mode)
{

	if (keys[GLFW_KEY_L])
	{
		if (play == false && (FrameIndex > 1))
		{

			resetElements();
			//First Interpolation				
			interpolation();

			play = true;
			playIndex = 0;
			i_curr_steps = 0;
		}
		else
		{
			play = false;
		}

	}

	if (keys[GLFW_KEY_K])
	{
		if (FrameIndex < MAX_FRAMES)
		{
			saveFrame();
		}

	}

	if (GLFW_KEY_ESCAPE == key && GLFW_PRESS == action)
	{
		glfwSetWindowShouldClose(window, GL_TRUE);
	}

	if (key >= 0 && key < 1024)
	{
		if (action == GLFW_PRESS)
		{
			keys[key] = true;
		}
		else if (action == GLFW_RELEASE)
		{
			keys[key] = false;
		}
	}

	if (keys[GLFW_KEY_SPACE])
	{
		active = !active;
		if (active)
			LightP1 = glm::vec3(1.0, 1.0, 1.0); //Enciende todo
		else
			LightP1 = glm::vec3(0.0f, 0.0f, 0.0f); //Apaga
	}
}

void MouseCallback(GLFWwindow *window, double xPos, double yPos)
{
	if (firstMouse)
	{
		lastX = xPos;
		lastY = yPos;
		firstMouse = false;
	}

	GLfloat xOffset = xPos - lastX;
	GLfloat yOffset = lastY - yPos;  // Reversed since y-coordinates go from bottom to left

	lastX = xPos;
	lastY = yPos;

	camera.ProcessMouseMovement(xOffset, yOffset);
}

