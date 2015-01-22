

#define GLEW_STATIC
#include "GL/glew.h"

#include "GLFW/glfw3.h"
#include <thread>

const int SCREEN_WIDTH = 1024;
const int SCREEN_HEIGHT = 768;

GLFWwindow* window;

void Initialize();
bool Quit();
void Destroy();
void Render();
void HandleUI();

int main()
{
	Initialize();

	//test GLEW
	GLuint vertexBuffer;
	glGenBuffers(1, &vertexBuffer);

	printf("%u\n", vertexBuffer);

	while (!glfwWindowShouldClose(window))
	{
		HandleUI();
		//render all drawn graphics to screen
		Render();
	}
	Destroy();
}

void Initialize()
{
	glfwInit();
	//create the window
	//windowed
	window = glfwCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "OpenGL Bitches!", nullptr, nullptr);
	//full screen
	//window = glfwCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "OpenGL Bitches!", glfwGetPrimaryMonitor(), nullptr);

	//make window active
	glfwMakeContextCurrent(window);

	//force GLEW to use a modern OpenGL method for checking if function is available
	glewExperimental = GL_TRUE;
	//initialize GLEW
	glewInit();


	
}

void Destroy()
{
	glfwTerminate();
}

void Render()
{
	glfwSwapBuffers(window);
	glfwPollEvents();
}

void HandleUI()
{
	//close window if 'ESC' key pressed
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);
}