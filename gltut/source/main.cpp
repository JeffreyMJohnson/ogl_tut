

#define GLEW_STATIC
#include "GL/glew.h"

#include "GLFW/glfw3.h"
#include "SOIL/SOIL.h"
#include <string>
#include <iostream>
#include <time.h>

const int SCREEN_WIDTH = 1024;
const int SCREEN_HEIGHT = 768;

GLFWwindow* window;

void Initialize();
bool Quit();
void Destroy();
void Render();
void HandleUI();
void CheckShaderCompileStatus(GLuint& vertexShader);

//vertex shader
/*
vertex shadder is program on graphics card that processes each vertex and it's attributes as they appear in the vertex array.
It's duty is to output the final vertex position in device coords and to output any data the fragment shader requires.
The 3D transform should take place here.
*/
const char* vertexShaderSource =
"#version 150 core\n"
"in vec2 position;"
"in vec3 color;"
"in vec2 texcoord;"
"out vec3 Color;"
"out vec2 TexCoord;"
"void main()"
"{"
"Color = color;"
"TexCoord = texcoord;"
"gl_Position = vec4(position.x, position.y, 0.0, 1.0);"
"}";

//fragment shader
/*
The output of the vertex shader is iterpolated over all the pixels on the screen covered by a primitive.  These pixels are called fragments
and this is what the fragment shader operates on.  Just likle vertex shader, the fragment shader has one mandatory output, the final color
of a fragment.
NOTE: colors in OpenGL are usually represented as float from 0 - 1 instead of common 0 - 255.
*/
const char* fragmentShaderSource =
"#version 150 core\n"
"in vec3 Color;"
"in vec2 TexCoord;"
"out vec4 outColor;"
"uniform sampler2D texKitten;"
"uniform sampler2D texPuppy;"
"uniform float time;"
"void main()"
"{"
"if (TexCoord.y < .5)"
"	outColor = texture(texKitten, TexCoord);"
"else"
"	outColor = texture(texKitten, vec2(TexCoord.x + sin(TexCoord.y * 60 + time * 2.0) / 30, 1 - TexCoord.y)) * vec4(.7,.7,1,1);"
//"	outColor = texture(texKitten, vec2(TexCoord.x, 1.0 - TexCoord.y));"
//"outColor = mix(texture(texKitten, TexCoord), texture(texPuppy, TexCoord), time);"
"}";


int main()
{
	//initialize GLEW and GLFW for window
	Initialize();


	/***************************************************************************************************************************************/
	/*											vertex array objects																	   */
	/***************************************************************************************************************************************/
	/*
	A Vertex array object  stores all of the links between attributes and VBO's raw vertex data.

	as soon as VAO is bound, every time you call glVertexAttribPointer, that information is stored in that VAO. This makes switching between
	different vertex data and vertex formats as easy as binding a differentg vao.  Note, a VAO doesn't store any vertex data itself, it just
	references the VBO's created and how to retrieve attribute values from them.  Since only calls after binding a vao stick to it, make
	sure you create and bind vao at start of program.
	*/
	//create vertex array object
	GLuint vao;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	/***************************************************************************************************************************************/
	/*											vertex buffer objects																	   */
	/***************************************************************************************************************************************/

	//load vertices into GPU via vertex buffer object
	GLuint vbo;//handle to buffer object, need to make active and destroy when done
	glGenBuffers(1, &vbo);//get the handle from OpenGL



	//vertex data including position (2), color (3), and texture Coordinates (2)
	float vertices[] =
	{
		-.5, .5, 1, 0, 0, 0, 0, //top-left
		.5, .5, 0, 1, 0, 1, 0,//top-right
		.5, -.5, 0, 0, 1, 1, 1,//bottom-right
		-.5, -.5, 1, 1, 1, 0, 1//bottom-left
	};

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
	/*											element buffers																			   */
	/***************************************************************************************************************************************/
	/*
	way to control the order, which also enables you to reuse existing vertices.
	*/

	//create element array
	GLuint ebo;
	glGenBuffers(1, &ebo);

	//refers to the order to draw the  vertices bound to GL_ARRAY_BUFFER
	GLuint elements[] =
	{
		0, 1, 2,
		2, 3, 0
	};


	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);

	/*
	creates and initializes a buffer object's data store
	params
	target - Specifies the target to which the buffer object is bound for glBufferData
	size - Specifies the size in bytes of the buffer object's new data store.
	data - Specifies a pointer to data that will be copied into the data store for initialization, or NULL if no data is to be copied.
	usage - Specifies the expected usage pattern of the data store.
	*/
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(elements), elements, GL_STATIC_DRAW);



	/***************************************************************************************************************************************/
	/*											shader creation, compile																   */
	/***************************************************************************************************************************************/

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

	//enable the vertex attribute array
	glEnableVertexAttribArray(positionAttribute);

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
	//stride is size of each vertex = 5 * floats per
	glVertexAttribPointer(positionAttribute, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 7, 0);


	GLint colAttrib = glGetAttribLocation(shaderProgram, "color");
	glEnableVertexAttribArray(colAttrib);
	//offset per vertex is 2 * float to get to color data
	glVertexAttribPointer(colAttrib, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 7, (void*)(sizeof(float) * 2));

	//values of uniform are changed with any of glUniformXY functions, where X is number of components, and Y is the type (eg (f)loat, (d)ouble and (i)nteger
	/*
	glUniform — Specify the value of a uniform variable for the current program object
	param
	location - location of uniform in shader
	value0 - red value (in this context)
	value1 - green value (in this context)
	value2 - blue value (in this context)
	*/
	//glUniform3f(uniColor, 1, 0, 0);

	//texture data
	GLint texAttrib = glGetAttribLocation(shaderProgram, "texcoord");
	glEnableVertexAttribArray(texAttrib);
	glVertexAttribPointer(texAttrib, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 7, (void*)(sizeof(float) * 5));


	/***************************************************************************************************************************************/
	/*											Texture objects and paramaters															   */
	/***************************************************************************************************************************************/
	GLuint textures[2];
	glGenTextures(2, textures);

	//load texture
	//GLuint tex;
	//glGenTextures(1, &tex);

	//need to load the image into memory before configuring texture params, using SOIL for loading image
	int width, height;
	unsigned char* image;//= SOIL_load_image(".\\images\\sample.png", &width, &height, 0, SOIL_LOAD_RGB);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, textures[0]);
	image = SOIL_load_image(".\\images\\sample.png", &width, &height, 0, SOIL_LOAD_RGB);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
	SOIL_free_image_data(image);
	glUniform1i(glGetUniformLocation(shaderProgram, "texKitten"), 0);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, textures[1]);
	image = SOIL_load_image(".\\images\\sample2.png", &width, &height, 0, SOIL_LOAD_RGB);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
	SOIL_free_image_data(image);

	glUniform1i(glGetUniformLocation(shaderProgram, "texPuppy"), 1);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	double deltaTime = 0.0;
	double step = .001;
	bool increase = true;
	GLint u_time = glGetUniformLocation(shaderProgram, "time");

	while (!glfwWindowShouldClose(window))
	{
		HandleUI();
		//clear screen to black
		glClearColor(0, 0, 0, 1);
		glClear(GL_COLOR_BUFFER_BIT);
		/*
		params
		specifies kind of primitive
		specifies how many vertices to skip at the beginning
		specifies the number of vertices to process
		*/
		//glDrawArrays(GL_TRIANGLES, 0, 6);

		if (increase)
		{
			deltaTime += step;
		}
		else
		{
			deltaTime -= step;
		}


		if (deltaTime > 1.0)
		{
			increase = false;
		}
		if (deltaTime < 0)
		{
			increase = true;
		}

		glUniform1f(u_time, deltaTime);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);


		//render all drawn graphics to screen
		Render();
	}
	glDeleteTextures(2, textures);

	glDeleteProgram(shaderProgram);
	glDeleteShader(fragmentShader);
	glDeleteShader(vertexShader);

	glDeleteBuffers(1, &ebo);
	glDeleteBuffers(1, &vbo);

	glDeleteVertexArrays(1, &vao);

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