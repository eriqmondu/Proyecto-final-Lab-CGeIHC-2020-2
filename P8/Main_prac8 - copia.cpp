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

// Prototipos de funciones
void KeyCallback(GLFWwindow *window, int key, int scancode, int action, int mode);
void MouseCallback(GLFWwindow *window, double xPos, double yPos);
void DoMovement();
void LoadTextures(void);
unsigned int generateTextures(char*, bool);

// Window dimensions
const GLuint WIDTH = 800, HEIGHT = 600;
int SCREEN_WIDTH, SCREEN_HEIGHT;

// Camera
Camera  camera(glm::vec3(0.0f, 0.0f, 3.0f));
GLfloat lastX = WIDTH / 2.0;
GLfloat lastY = HEIGHT / 2.0;
bool keys[1024];
bool firstMouse = true;
float range = 0.0f;
float spotAngle = 0.0f;

// Light attributes
glm::vec3 lightPos(0.0f, -5.0f, 0.0f);
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
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_2D, textureID);
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
}

int main()
{
	// Init GLFW
	glfwInit();
	// Set all the required options for GLFW
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
	glfwWindowHint(GLFW_SAMPLES, 4);

	// Create a GLFWwindow object that we can use for GLFW's functions
	GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "ff 8", nullptr, nullptr);

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

	//Carga de texturas primitivas (esto iba antes de glEnable
	LoadTextures();

	// Build and compile our shader program


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

	GLuint indices[] =
	{  // Note that we start from 0!
		0,1,3,
		1,2,3,

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

	////// Definición de texturas para primitivas
	////GLuint specular_map, texture0, texture1, texture2, texture3, texture4, texture5, texture6, texture7, texture8, texture9;
	//glGenTextures(1, &specular_map);
	//glGenTextures(1, &texture0);
	//glGenTextures(1, &texture1);
	//glGenTextures(1, &texture2);
	//glGenTextures(1, &texture3);
	//glGenTextures(1, &texture4);
	//glGenTextures(1, &texture5);
	//glGenTextures(1, &texture6);
	//glGenTextures(1, &texture7);
	//glGenTextures(1, &texture8);
	//glGenTextures(1, &texture9);

	//int textureWidth, textureHeight, nrChannels;
	//stbi_set_flip_vertically_on_load(true);
	//unsigned char* image;

	//// Carga del mapa especular blanco
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST_MIPMAP_NEAREST);
	//image = stbi_load("images/mapa_blanco.jpg", &textureWidth, &textureHeight, &nrChannels, 0);
	//glBindTexture(GL_TEXTURE_2D, specular_map);
	//glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, textureWidth, textureHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
	//glGenerateMipmap(GL_TEXTURE_2D);
	//if (image)
	//{
	//	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, textureWidth, textureHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
	//	glGenerateMipmap(GL_TEXTURE_2D);
	//}
	//else
	//{
	//	std::cout << "Failed to load texture" << std::endl;
	//}
	//stbi_image_free(image);

	////Carga textura suelo de madera
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST_MIPMAP_NEAREST);
	//image = stbi_load("images/floor.jpg", &textureWidth, &textureHeight, &nrChannels, 0);
	//glBindTexture(GL_TEXTURE_2D, texture0);
	//glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, textureWidth, textureHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
	//glGenerateMipmap(GL_TEXTURE_2D);
	//if (image)
	//{
	//	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, textureWidth, textureHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
	//	glGenerateMipmap(GL_TEXTURE_2D);
	//}
	//else
	//{
	//	std::cout << "Failed to load texture" << std::endl;
	//}
	//stbi_image_free(image);

	//// Carga textura tapete
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST_MIPMAP_NEAREST);
	//image = stbi_load("images/tapete.jpg", &textureWidth, &textureHeight, &nrChannels, 0);
	//glBindTexture(GL_TEXTURE_2D, texture1);
	//glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, textureWidth, textureHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
	//glGenerateMipmap(GL_TEXTURE_2D);
	//if (image)
	//{
	//	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, textureWidth, textureHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
	//	glGenerateMipmap(GL_TEXTURE_2D);
	//}
	//else
	//{
	//	std::cout << "Failed to load texture" << std::endl;
	//}
	//stbi_image_free(image);

	//// Carga textura pared
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST_MIPMAP_NEAREST);
	//image = stbi_load("images/pared.jpg", &textureWidth, &textureHeight, &nrChannels, 0);
	//glBindTexture(GL_TEXTURE_2D, texture2);
	//glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, textureWidth, textureHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
	//glGenerateMipmap(GL_TEXTURE_2D);
	//if (image)
	//{
	//	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, textureWidth, textureHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
	//	glGenerateMipmap(GL_TEXTURE_2D);
	//}
	//else
	//{
	//	std::cout << "Failed to load texture" << std::endl;
	//}
	//stbi_image_free(image);

	//// Carga textura ventana
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST_MIPMAP_NEAREST);
	//image = stbi_load("images/window.png", &textureWidth, &textureHeight, &nrChannels, 0);
	//glBindTexture(GL_TEXTURE_2D, texture3);
	//glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, textureWidth, textureHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
	//glGenerateMipmap(GL_TEXTURE_2D);
	//if (image)
	//{
	//	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, textureWidth, textureHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
	//	glGenerateMipmap(GL_TEXTURE_2D);
	//}
	//else
	//{
	//	std::cout << "Failed to load texture" << std::endl;
	//}
	//stbi_image_free(image);

	//// Carga cuadro 1
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST_MIPMAP_NEAREST);
	//image = stbi_load("images/cuadro1.jpg", &textureWidth, &textureHeight, &nrChannels, 0);
	//glBindTexture(GL_TEXTURE_2D, texture4);
	//glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, textureWidth, textureHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
	//glGenerateMipmap(GL_TEXTURE_2D);
	//if (image)
	//{
	//	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, textureWidth, textureHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
	//	glGenerateMipmap(GL_TEXTURE_2D);
	//}
	//else
	//{
	//	std::cout << "Failed to load texture" << std::endl;
	//}
	//stbi_image_free(image);

	//// Carga cuadro 2
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST_MIPMAP_NEAREST);
	//image = stbi_load("images/cuadro2.jpg", &textureWidth, &textureHeight, &nrChannels, 0);
	//glBindTexture(GL_TEXTURE_2D, texture5);
	//glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, textureWidth, textureHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
	//glGenerateMipmap(GL_TEXTURE_2D);
	//if (image)
	//{
	//	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, textureWidth, textureHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
	//	glGenerateMipmap(GL_TEXTURE_2D);
	//}
	//else
	//{
	//	std::cout << "Failed to load texture" << std::endl;
	//}
	//stbi_image_free(image);

	//// Carga vidrios de la ventana
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST_MIPMAP_NEAREST);
	//image = stbi_load("images/window_front.png", &textureWidth, &textureHeight, &nrChannels, 0);
	//glBindTexture(GL_TEXTURE_2D, texture6);
	//glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, textureWidth, textureHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
	//glGenerateMipmap(GL_TEXTURE_2D);
	//if (image)
	//{
	//	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, textureWidth, textureHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
	//	glGenerateMipmap(GL_TEXTURE_2D);
	//}
	//else
	//{
	//	std::cout << "Failed to load texture" << std::endl;
	//}
	//stbi_image_free(image);

	//// Carga foto mamá CJ
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST_MIPMAP_NEAREST);
	//image = stbi_load("images/cj_mom.jpg", &textureWidth, &textureHeight, &nrChannels, 0);
	//glBindTexture(GL_TEXTURE_2D, texture7);
	//glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, textureWidth, textureHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
	//glGenerateMipmap(GL_TEXTURE_2D);
	//if (image)
	//{
	//	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, textureWidth, textureHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
	//	glGenerateMipmap(GL_TEXTURE_2D);
	//}
	//else
	//{
	//	std::cout << "Failed to load texture" << std::endl;
	//}
	//stbi_image_free(image);

	//// Carga puerta blanca
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST_MIPMAP_NEAREST);
	//image = stbi_load("images/puerta_blanca.jpg", &textureWidth, &textureHeight, &nrChannels, 0);
	//glBindTexture(GL_TEXTURE_2D, texture8);
	//glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, textureWidth, textureHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
	//glGenerateMipmap(GL_TEXTURE_2D);
	//if (image)
	//{
	//	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, textureWidth, textureHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
	//	glGenerateMipmap(GL_TEXTURE_2D);
	//}
	//else
	//{
	//	std::cout << "Failed to load texture" << std::endl;
	//}s
	//stbi_image_free(image);

	//// Carga puerta café
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST_MIPMAP_NEAREST);
	//image = stbi_load("images/puerta_cafe.jpg", &textureWidth, &textureHeight, &nrChannels, 0);
	//glBindTexture(GL_TEXTURE_2D, texture9);
	//glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, textureWidth, textureHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
	//glGenerateMipmap(GL_TEXTURE_2D);
	//if (image)
	//{
	//	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, textureWidth, textureHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
	//	glGenerateMipmap(GL_TEXTURE_2D);
	//}
	//else
	//{
	//	std::cout << "Failed to load texture" << std::endl;
	//}
	//stbi_image_free(image);



	glBindTexture(GL_TEXTURE_2D, 0);

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

		// Clear the colorbuffer
		glClearColor(0.3f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	    //Load Model
	

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
		glBindVertexArray(VAO);

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
		// Swap the screen buffers
		glfwSwapBuffers(window);
	}

	glDeleteVertexArrays(1, &VAO);
	glDeleteVertexArrays(1, &lightVAO);
	glDeleteBuffers(1, &VBO);
	glDeleteBuffers(1, &EBO);
	// Terminate GLFW, clearing any resources allocated by GLFW.
	glfwTerminate();



	return 0;
}

// Moves/alters the camera positions based on user input
void DoMovement()
{
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