#include <iostream>
#include <vector>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "ImGui/imgui.h"
#include "ImGui/imgui_impl_glfw.h"
#include "ImGui/imgui_impl_opengl3.h"



//Variables globales para el estado del paint
enum herramienta { Ninguna, Linea, Circulo};
herramienta herramientaActual = Linea; //Inicia seleccionando la linea por defecto

int estadoClick = 0;

//Puntos de control interactivos
int clickX0 = 0, clickY0 = 0; 
int clickX1 = 0, clickY1 = 0; 


float escalaGrid = 0.1f;
bool actualizarBuffers = false;

std::vector<float> lineaVertices;
std::vector<float> circuloVertices;
GLuint lineaVAO, lineaVBO;
GLuint circuloVAO, circuloVBO;

//Declaracion de funciones
std::vector<float> generarLineasBresenham(int x0, int y0, int x1, int y1, float escala);
std::vector<float>generarCirculoBresenham(int cx, int cy, int r, float escala);


// CalBack para actualizar el viewport al redimensionar la ventana
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}

// Interaccion con el teclado: cambiar de herramientas con 'l' y 'c' para usar el paint
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (action == GLFW_PRESS) {
		if (key == GLFW_KEY_L) {
			herramientaActual = Linea;
			estadoClick = 0;
			std::cout << "Herramienta activa: LINEA(USA 2 CLICKS EN LA GRILLA)" << std::endl;
		}
		if (key == GLFW_KEY_C) {
			herramientaActual = Circulo;
			estadoClick = 0;
			std::cout << "Herramienta activa: CIRCULO (Click 1: centro, click 2 Radio) " << std::endl;
		}
	}
}

//Interaccion con el raton: captura de clicks y conversion a coordenadas de la grilla 
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
	if(ImGui::GetIO().WantCaptureMouse) {
		return;
	}
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
	{
		double mouseX, mouseY;
		glfwGetCursorPos(window, &mouseX, &mouseY);

		int width, height;
		glfwGetWindowSize(window, &width, &height);
		if (height == 0) height = 1;

		// 1. Transformar coordenadas de pantalla de GLFW a Espacio de Proyección Normalizado [-1, 1]
		float ndcX = (2.0f * mouseX) / width - 1.0f;
		float ndcY = 1.0f - (2.0f * mouseY) / height;

		// 2. Aplicar la inversa de tu matriz ortográfica manual (Multiplicar por r y t)
		float aspectRatio = (float)width / (float)height;
		float orthoHeight = 1.0f;
		float orthoWidth = orthoHeight * aspectRatio;

		float worldX = ndcX * orthoWidth;
		float worldY = ndcY * orthoHeight;

		// 3. Convertir de coordenadas del mundo a índices enteros de tu cuadrícula
		int gridX = std::floor(worldX / escalaGrid);
		int gridY = std::floor(worldY / escalaGrid);

		// Lógica de estados para el dibujo interactivo
		if (estadoClick == 0) {
			clickX0 = gridX;
			clickY1 = gridY;
			estadoClick = 1;
			std::cout << "Primer Click en: (" << clickX0 << ", " << ").Esperando segundo click..." << std::endl;

		}
		else if (estadoClick == 1) {
			clickX1 = gridX;
			clickY1 = gridY;
			estadoClick = 0; //Reiniciar flujo
			std::cout << "Segundo Click en (" << clickX1 << ", " << clickY1 << ")" << std::endl;
			actualizarBuffers = true;
		}
	}
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
//Funcion de Bresenham paar circulo de celdas rellenas
std::vector<float> generarCirculoBresenham(int cx, int cy, int r, float escala) {
	std::vector<float> vertices;

	int x = 0;
	int y = r;
	int d = 3 - 2 * r;

	//Funcion interna para reutilizar la logica de dibujar el cuadrado en cada celda
	auto agregarCuadrado = [&](int xc, int yc) {
		float xMin = xc * escala;
		float xMax = (xc + 1) * escala;
		float yMin = yc * escala;
		float yMax = (yc + 1) * escala;

		//Triangulo 1
		vertices.push_back(xMin); vertices.push_back(yMin); vertices.push_back(0.0f);
		vertices.push_back(xMin); vertices.push_back(yMax); vertices.push_back(0.0f);
		vertices.push_back(xMax); vertices.push_back(yMin); vertices.push_back(0.0f);

		//Triangulo2
		vertices.push_back(xMin); vertices.push_back(yMax); vertices.push_back(0.0f);
		vertices.push_back(xMax); vertices.push_back(yMax); vertices.push_back(0.0f);
		vertices.push_back(xMax); vertices.push_back(yMin); vertices.push_back(0.0f);
		
		};
	// 8 puntos simetricos iniciales
	while (y >= x) {
		// simetria de 8 vias
		agregarCuadrado(cx + x, cy + y);
		agregarCuadrado(cx - x, cy + y);
		agregarCuadrado(cx + x, cy - y);
		agregarCuadrado(cx - x, cy - y);
		agregarCuadrado(cx + y, cy + x);
		agregarCuadrado(cx - y, cy + x);
		agregarCuadrado(cx + y, cy - x);
		agregarCuadrado(cx - y, cy - x);

		x++;

		//Actualizar el parametro de decision
		if (d > 0) {
			y--;
			d = d + 4 * (x - y) + 10;
		}
		else {
			d = d + 4 * x + 6;
		}

	}
	return vertices;

}


//Funcion principal
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

	//CONFIGURACION IMGUI
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;

	//Configurar estilo visual
	ImGui::StyleColorsDark();

	//Iniciar backends para glfw y openGL
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 330 core");

	// Registrar callback de tamaño // se agrego callback interactivo
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetMouseButtonCallback(window, mouse_button_callback);
	glfwSetKeyCallback(window, key_callback);

	std::cout << " -- Control del paint --" << std::endl;
	std::cout << "Presiona 'L' para linea " << std::endl;
	std::cout << "Presion 'C' para modo circulo" << std::endl;


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
		// Líneas verticales
		gridVertices.push_back(i); gridVertices.push_back(-maxCoord); gridVertices.push_back(0.0f);
		gridVertices.push_back(i); gridVertices.push_back(maxCoord); gridVertices.push_back(0.0f);

		// Líneas horizontales
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

	//Crear buffers vacios iniciales para la linea interactiva
	glGenVertexArrays(1, &lineaVAO); glGenBuffers(1, &lineaVBO);
	glBindVertexArray(lineaVAO); glBindBuffer(GL_ARRAY_BUFFER, lineaVBO);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	//Crear Buffers vacios iniciales para el circulo interactivo
	glGenVertexArrays(1, &circuloVAO); glGenBuffers(1, &circuloVBO);
	glBindVertexArray(circuloVAO); glBindBuffer(GL_ARRAY_BUFFER, circuloVBO);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);


	// BUCLE DE RENDERIZADO
		// BUCLE DE RENDERIZADO (Reemplaza desde tu línea 315 en adelante)
	while (!glfwWindowShouldClose(window))
	{
		glfwPollEvents();

		// 1. INICIAR EL CONTEXTO DE IMGUI INMEDIATAMENTE (Mover esto aquí arriba es el truco)
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		// 2. DISEÑAR TU VENTANA FLOTANTE DE IMGUI
		ImGui::Begin("Herramientas del Paint");
		if (ImGui::Button("Modo Linea")) { herramientaActual = Linea; estadoClick = 0; }
		if (ImGui::Button("Modo Circulo")) { herramientaActual = Circulo; estadoClick = 0; }
		ImGui::Text("Herramienta activa: %s", (herramientaActual == Linea) ? "LINEA" : "CIRCULO");
		ImGui::End();

		// 3. LIMPIAR PANTALLA
		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		// 4. ACTUALIZACIÓN DE BUFFERS DE BRESENHAM (Tus líneas 321 a 335)
		if (actualizarBuffers) {
			if (herramientaActual == Linea) {
				lineaVertices = generarLineasBresenham(clickX0, clickY0, clickX1, clickY1, escalaGrid);
				glBindBuffer(GL_ARRAY_BUFFER, lineaVBO);
				glBufferData(GL_ARRAY_BUFFER, lineaVertices.size() * sizeof(float), lineaVertices.data(), GL_STATIC_DRAW);
			}
			else if (herramientaActual == Circulo) {
				int radio = std::round(std::sqrt(std::pow(clickX1 - clickX0, 2) + std::pow(clickY1 - clickY0, 2)));
				circuloVertices = generarCirculoBresenham(clickX0, clickY0, radio, escalaGrid);
				glBindBuffer(GL_ARRAY_BUFFER, circuloVBO);
				glBufferData(GL_ARRAY_BUFFER, circuloVertices.size() * sizeof(float), circuloVertices.data(), GL_STATIC_DRAW);
			}
			actualizarBuffers = false;
		}

		// 5. OBTENER TAMAÑO Y ENVIAR MATRIZ PROJECTION
		int width, height;
		glfwGetFramebufferSize(window, &width, &height);
		if (height == 0) height = 1;
		glViewport(0, 0, width, height); // Mantiene alineada la pantalla al estirar

		float aspectRatio = (float)width / (float)height;
		float orthoHeight = 1.0f;
		float orthoWidth = orthoHeight * aspectRatio;
		float r = orthoWidth;
		float t = orthoHeight;
		float projection[16] = {
			1.0f / r, 0.0f,     0.0f,  0.0f,
			0.0f,     1.0f / t, 0.0f,  0.0f,
			0.0f,     0.0f,    -1.0f,  0.0f,
			0.0f,     0.0f,     0.0f,  1.0f
		};

		glUseProgram(shaderProgram);
		GLuint projloc = glGetUniformLocation(shaderProgram, "projection");
		glUniformMatrix4fv(projloc, 1, GL_FALSE, projection);

		GLuint colorLoc = glGetUniformLocation(shaderProgram, "objectColor");

		// 6. DIBUJAR LA GRILLA BLANCA
		glUniform4f(colorLoc, 1.0f, 1.0f, 1.0f, 1.0f);
		glBindVertexArray(VAO);
		glDrawArrays(GL_LINES, 0, gridVertices.size() / 3);

		// 7. DIBUJAR LA LÍNEA ROJA
		if (!lineaVertices.empty()) {
			glUniform4f(colorLoc, 1.0f, 0.0f, 0.0f, 1.0f);
			glBindVertexArray(lineaVAO);
			glDrawArrays(GL_TRIANGLES, 0, lineaVertices.size() / 3);
		}

		// 8. DIBUJAR EL CÍRCULO AZUL
		if (!circuloVertices.empty()) {
			glUniform4f(colorLoc, 0.0f, 0.5f, 1.0f, 1.0f);
			glBindVertexArray(circuloVAO);
			glDrawArrays(GL_TRIANGLES, 0, circuloVertices.size() / 3);
		}

		// 9. ENVIAR LOS DATOS DE IMGUI A DIBUJAR (Al final de todo el pipeline gráfico)
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		// 10. REFRESCAR MONITOR
		glfwSwapBuffers(window);
	}



	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);

	//Limpieza
	glDeleteVertexArrays(1, &VAO); glDeleteBuffers(1, &VBO);
	glDeleteVertexArrays(1, &lineaVAO); glDeleteBuffers(1, &lineaVBO);
	glDeleteVertexArrays(1, &circuloVAO); glDeleteBuffers(1, &circuloVBO);
	glDeleteProgram(shaderProgram);
	glfwDestroyWindow(window);
	glfwTerminate();
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	return 0;
}
