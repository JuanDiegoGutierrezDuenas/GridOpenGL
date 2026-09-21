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
" gl_Position = projection * vec4(aPos, 1.0);\n"
"}\0";

//Fragment Shader Source Code
const char* fragmentShaderSource = "#version 330 core\n"
"out vec4 FragColor;\n"
"uniform vec4 objectColor;\n" // <- CORREGIDO: Todo en minúsculas para evitar conflictos
"void main()\n"
"{\n"
" FragColor = objectColor;\n"
"}\n\0";

// Funcion de Bresenham Modificada para alinear los cuadrados dentro de la grilla
std::vector<float> generarLineasBresenham(int x0, int y0, int x1, int y1, float escala) {
	std::vector<float> vertices;
	int dx = abs(x1 - x0), dy = abs(y1 - y0);
	int sx = (x0 < x1) ? 1 : -1, sy = (y0 < y1) ? 1 : -1;
	int err = dx - dy;

	while (true) {
		// CORRECCIÓN DE ALINEACIÓN: 
		// Ahora cx y cy definen la esquina inferior izquierda de la celda de la grilla,
		// en lugar del centro matemático desfasado.
		float xMin = x0 * escala;
		float xMax = (x0 + 1) * escala;
		float yMin = y0 * escala;
		float yMax = (y0 + 1) * escala;

		// Triángulo 1 (Abajo-Izquierda, Arriba-Izquierda, Abajo-Derecha)
		vertices.push_back(xMin); vertices.push_back(yMin); vertices.push_back(0.0f);
		vertices.push_back(xMin); vertices.push_back(yMax); vertices.push_back(0.0f);
		vertices.push_back(xMax); vertices.push_back(yMin); vertices.push_back(0.0f);

		// Triángulo 2 (Arriba-Izquierda, Arriba-Derecha, Abajo-Derecha)
		vertices.push_back(xMin); vertices.push_back(yMax); vertices.push_back(0.0f);
		vertices.push_back(xMax); vertices.push_back(yMax); vertices.push_back(0.0f);
		vertices.push_back(xMax); vertices.push_back(yMin); vertices.push_back(0.0f);

		if (x0 == x1 && y0 == y1) break;
		int e2 = 2 * err;
		if (e2 > -dy) { err -= dy; x0 += sx; }
		if (e2 < dx) { err += dx; y0 += sy; }
	}
	return vertices;
}


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

	// Buffers de la linea 
	float escalaGrid = 0.1f;
	std::vector<float> lineaVertices = generarLineasBresenham(-3, -3, 3, 3, escalaGrid);// SE PUEDE CAMBIAR LA POSICION DE LA LINEA DE BRESENHAM AQUI//(x0, y0) es el inicio, (x1, y1) es el final

	GLuint lineaVAO, lineaVBO;
	glGenVertexArrays(1, &lineaVAO);
	glGenBuffers(1, &lineaVBO);
	glBindVertexArray(lineaVAO);
	glBindBuffer(GL_ARRAY_BUFFER, lineaVBO);
	glBufferData(GL_ARRAY_BUFFER, lineaVertices.size() * sizeof(float), lineaVertices.data(), GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);


	// BUCLE DE RENDERIZADO
	while (!glfwWindowShouldClose(window))
	{
		glfwPollEvents();
		//Limpiar pantalla
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

		float r = orthoWidth;
		float t = orthoHeight;

		// CORREGIDO: Faltaba el [16] para que sea una matriz real de 4x4
		float projection[16] = {
			1.0f / r, 0.0f,     0.0f,  0.0f,
			0.0f,     1.0f / t, 0.0f,  0.0f,
			0.0f,     0.0f,    -1.0f,  0.0f,
			0.0f,     0.0f,     0.0f,  1.0f
		};

		// Envia la matriz corregida al shader
		GLuint projloc = glGetUniformLocation(shaderProgram, "projection");
		glUniformMatrix4fv(projloc, 1, GL_FALSE, projection);

		// Obtener ubicación del uniform del color
		GLuint colorLoc = glGetUniformLocation(shaderProgram, "objectColor");

		// DIBUJAR LA GRILLA EN BLANCO
		glUniform4f(colorLoc, 1.0f, 1.0f, 1.0f, 1.0f); // Blanco puro
		glBindVertexArray(VAO);
		glDrawArrays(GL_LINES, 0, gridVertices.size() / 3);

		// DIBUJAR LOS CUADRADOS DE BRESENHAM EN ROJO
		glUniform4f(colorLoc, 1.0f, 0.0f, 0.0f, 1.0f); // Rojo puro
		glBindVertexArray(lineaVAO);

		// Dibuja los triángulos de la función modificada
		glDrawArrays(GL_TRIANGLES, 0, lineaVertices.size() / 3);

		glfwSwapBuffers(window);
	}


	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);

	//Limpieza
	glDeleteVertexArrays(1, &lineaVAO);
	glDeleteBuffers(1, &lineaVBO);

	glDeleteProgram(shaderProgram);

	glfwDestroyWindow(window);
	glfwTerminate();
	return 0;
}
