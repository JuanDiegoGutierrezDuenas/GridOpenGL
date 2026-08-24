#include <iostream>
#include <vector>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

//Vertex Shader Source Code
const char* vertexShaderSource = "#version 330 core\n"
"layout (location = 0) in vec3 aPos;\n"
"uniform mat4 projection;\n"
"void main()\n"
"{\n"
"  gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
"}\0";
//Fragment Shader Source Code
const char* fragmentShaderSource = "#version 330 core\n"
"out vec4 FragColor;\n"
"void main()\n"
"{\n"
" FragColor = vec4(1.0f, 1.0f, 1.0f, 1.0f);\n"
"}\n\0";

int main()
{
	// GLFW CONFIG
	glfwInit();

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);


	// CREATE Window
	GLFWwindow* window = glfwCreateWindow(800, 600, "GridOpenGL", NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return-1;
	}
	glfwMakeContextCurrent(window);
	//GLAD CONFIG
	gladLoadGL();

	glViewport(0, 0, 800, 600);

	//SHADERS COMPILATION
	GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
	glCompileShader(vertexShader);

	GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER); 
	glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
	glCompileShader(fragmentShader);

	GLuint shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);
	glLinkProgram(shaderProgram);

	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

	//--VERTEX OF THE GRID
	std::vector<float> gridVertices;
	float step = 0.1f;

	//Lineas horizontales y verticales
	for (float i = -1.0f; i <= 1.0f; i += step) {

		gridVertices.push_back(i); gridVertices.push_back(-1.0f); gridVertices.push_back(0.0f);
		gridVertices.push_back(i); gridVertices.push_back(1.0f); gridVertices.push_back(0.0f);

		gridVertices.push_back(-1.0f); gridVertices.push_back(i); gridVertices.push_back(0.0f);
		gridVertices.push_back(1.0f); gridVertices.push_back(i); gridVertices.push_back(0.0f);
	
	
	}

	//Configuracion de VAO y VBO
	GLuint VAO, VBO;
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);

	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, gridVertices.size() * sizeof(float), gridVertices.data(), GL_STATIC_DRAW);
	
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	// BUCLE DE RENDERIZADO
	while (!glfwWindowShouldClose(window))
	{
		glfwPollEvents();
		//Limpiar pnantalla
		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		// Activar los shader y el VAO
		glUseProgram(shaderProgram);
		glBindVertexArray(VAO);

		//Dibujar usando GL_Lines
		glDrawArrays(GL_LINES, 0, gridVertices.size() / 3);

		glfwSwapBuffers(window);
	}

	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);
	glDeleteProgram(shaderProgram);

	glfwDestroyWindow(window);
	glfwTerminate();
	return 0;
}