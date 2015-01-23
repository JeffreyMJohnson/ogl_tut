

#define GLEW_STATIC
#include "GL/glew.h"

#include "GLFW/glfw3.h"
#include <string>
#include <iostream>

const int SCREEN_WIDTH = 1024;
const int SCREEN_HEIGHT = 768;

GLFWwindow* window;

void Initialize();
bool Quit();
void Destroy();
void Render();
void HandleUI();
void CheckShaderCompileStatus(GLuint& vertexShader);


int main()
{
	Initialize();

	/***************************************************************************************************************************************/
	/*											model to draw																			   */
	/***************************************************************************************************************************************/

	//triangle vertices in device coords (-1 to 1)
	float vertices[] =
	{
		0, .5,
		.5, -.5,
		-.5, -.5
	};

	/***************************************************************************************************************************************/
	/*											vertex array objects																	   */
	/***************************************************************************************************************************************/
	/*
	A Vertex array object  stores all of the links between attributes and VBO's raw vertex data.
	*/
	//bind it
	GLuint vao;
	glGenVertexArrays(1, &vao);

	/*
	as soon as VAO is bound, every time you call glVertexAttribPointer, that information is stored in that VAO. This makes switching between
	different vertex data and vertex formats as easy as binding a differentg vao.  Note, a VAO doesn't store any vertex data itself, it just
	references the VBO's created and how to retrieve attribute values from them.  Since only calls after binding a vao stick to it, make
	sure you create and bind vao at start of program.
	*/
	glBindVertexArray(vao);

	/***************************************************************************************************************************************/
	/*											vertex buffer objects																	   */
	/***************************************************************************************************************************************/

	//load vertices into GPU via vertex buffer object
	GLuint vbo;//handle to buffer object, need to make active and destroy when done
	glGenBuffers(1, &vbo);//get the handle from OpenGL

	//make it active array buffer
	glBindBuffer(GL_ARRAY_BUFFER, vbo);

	//now can copy the data to active array buffer on GPU
	/*note last param enum:
		GL_STATIC_DRAW - vertex data will be uploaded once and drawn many times (e.g. the world)
		GL_DYNAMIC_DRAW - vertex data will be changed from time to time, but drawn many times more than that.
		GL_STREAM_DRAW - vertex data will change almost every time it's drawn (e.g. user interface)
		*/
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);



	/***************************************************************************************************************************************/
	/*											shader creation, compile																   */		
	/***************************************************************************************************************************************/
	//vertex shader
	/*
	vertex shadder is program on graphics card that processes each vertex and it's attributes as they appear in the vertex array.
	It's duty is to output the final vertex position in device coords and to output any data the fragment shader requires.
	The 3D transform should take place here.
	*/
	char* vertexShaderSource =
		"#version 150\n" \
		"in vec2 position;\n" \
		"void main()\n" \
		"{\n" \
		"gl_Position = vec4(position, 0.0, 1.0);\n" \
		"}\n";

	//fragment shader
	/*
	The output of the vertex shader is iterpolated over all the pixels on the screen covered by a primitive.  These pixels are called fragments
	and this is what the fragment shader operates on.  Just likle vertex shader, the fragment shader has one mandatory output, the final color
	of a fragment.
	NOTE: colors in OpenGL are usually represented as float from 0 - 1 instead of common 0 - 255.
	*/
	char* fragmentShaderSource =
		"#version 150\n"\
		"out vec4 outColor;\n"\
		"void main()\n"\
		"{\n"\
		"outColor = vec4(1.0,1.0,1.0,1.0);\n"\
		"}\n";

	//compile shaders
	//create shader object and load data into it
	GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);

	////now compile it
	glCompileShader(vertexShader);

	//successful?
	CheckShaderCompileStatus(vertexShader);

	//now the fragment shader, same process
	GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);//get handle
	glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);//load source code
	glCompileShader(fragmentShader);//compile
	CheckShaderCompileStatus(fragmentShader);

	/***************************************************************************************************************************************/
	/*											shader program creation and linking														   */
	/***************************************************************************************************************************************/

	/*
	compile the shaders into a program
	shaders are designed to work together so need to link them via a program.
	*/
	GLuint shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);

	//fragment shader is allowed to write to multiple buffers, need to explicitly specify which output is written to which buffer.
	//this needs to happen before linking the program, but this is 0 by default and there is only one output so technically not necessary here.
	glBindFragDataLocation(shaderProgram, 0, "outColor");

	/*
	the connection is made by linking the program.  It is allowed to make changes to the shaders after they've been added to a program, but the actual
	result will not change until a program has been linked again.
	A shader object can be deleted with glDeleteShader, but it will not actually be removed before it has been detached from all programs with
	glDetachShader.
	*/
	glLinkProgram(shaderProgram);

	//to actually start using the shaders in the program
	glUseProgram(shaderProgram);
	//just like vertex buffer, only one program can be active at a time.

	/***************************************************************************************************************************************/
	/*											linking between vertex data and attributes												   */		
	/***************************************************************************************************************************************/
	//get reference to position input in vertex shader
	GLint positionAttribute = glGetAttribLocation(shaderProgram, "position");

	//with reference, specify how the data for that input is retrieved from the array
	/*
	params:
		references input
		specifies number of values for that input, same as number of components of the vec
		specifies the type of each component
		specifies whether input values should be normalized between -1 and 1 (or 0 and 1 depending on format
		specifies 'stride' (how many bytes are between each position attribute in array)
		specifies 'offset' (how many bytes from start of array the attribute occurs)
	*/
	/*
	NOTE:this function will store not only the stride and offset, but also the VBO that is currently bound to GL_ARRAY_BUFFER.
	That means that you don't have to explicitly bind the correct VBO when the actual draw functions are called.  This also 
	implies that can use a different VBO for each attribute.
	*/
	glVertexAttribPointer(positionAttribute, 2, GL_FLOAT, GL_FALSE, 0, 0);

	//enable the vertex attribute array
	glEnableVertexAttribArray(positionAttribute);





	while (!glfwWindowShouldClose(window))
	{
		HandleUI();

		/*
		params
			specifies kind of primitivsw
			specifies how many vertices to skip at the beginning
			specifies the number of vertices to process
		*/
		glDrawArrays(GL_TRIANGLES, 0, 3);

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

void CheckShaderCompileStatus(GLuint& shader)
{
	GLint status;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
	if (status == GL_FALSE)
	{
		GLchar buffer[512];
		glGetShaderInfoLog(shader, 512, NULL, buffer);
		printf("%s", buffer);
	}
	else
		printf("Shader compiled with no errors.\n");
}