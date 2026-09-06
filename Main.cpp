#include <iostream>
#include <vector>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

// CalBack para actualizar el viewport al redimensionar la ventana
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}

//Vertex Shader Source Code
const char* vertexShaderSource = "#version 330 core\n"
"layout (location = 0) in vec3 aPos;\n"
"uniform mat4 projection;\n"
"void main()\n"
"{\n"
" gl_Position = projection * vec4(aPos, 1.0);\n" // <- Cambio aquí
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

	// Registrar callback de tamaño
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

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

	//--VERTEX OF THE GRID// grid ampliado para cubrir pantalla al redimensionar
	std::vector<float> gridVertices;
	float step = 0.1f;
	float maxCoord = 10.0f;


	//Lineas horizontales y verticales
	for (float i = -maxCoord; i <= maxCoord; i += step) {

		//verticales
		gridVertices.push_back(i); gridVertices.push_back(-maxCoord); gridVertices.push_back(0.0f);
		gridVertices.push_back(i); gridVertices.push_back(maxCoord); gridVertices.push_back(0.0f);

		//Horizontales
		gridVertices.push_back(-maxCoord); gridVertices.push_back(i); gridVertices.push_back(0.0f);
		gridVertices.push_back(maxCoord); gridVertices.push_back(i); gridVertices.push_back(0.0f);
	
	
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

		// Obtener tamaño actual de la ventana
		int width, height;
		glfwGetFramebufferSize(window, &width, &height);
		if (height == 0) height = 1;

		float aspectRatio = (float)width / (float)height;
		float orthoHeight = 1.0f;
		float orthoWidth = orthoHeight * aspectRatio;
		//Matriz ortografica manual que va columna por columna
		// Mapea[-orthoWidth, orthoWidth] a[-1, 1] y[-orthoHeight, orthoHeight] a[-1, 1]
		float r = orthoWidth;
		float t = orthoHeight;
		float projection[16] = {
			1.0f / r, 0.0f,     0.0f,  0.0f,
			0.0f,     1.0f / t, 0.0f,  0.0f,
			0.0f,     0.0f,    -1.0f,  0.0f,
			0.0f,     0.0f,     0.0f,  1.0f
		};
		//Envia la matriz manual al shader
		GLuint projloc = glGetUniformLocation(shaderProgram, "projection");
		glUniformMatrix4fv(projloc, 1, GL_FALSE, projection);

		glBindVertexArray(VAO);
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