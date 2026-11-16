/*
------------------------------
PAINT BASADO EN OPENGL USANDO GLAD, GLFW Y IMGUI
*/
#include <iostream>
#include <vector>
#include <queue> //primero en entrar,´primero en salir 
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "ImGui/imgui.h"
#include "ImGui/imgui_impl_glfw.h"
#include "ImGui/imgui_impl_opengl3.h"
#include <cmath>

//Variables globales para el estado del paint
//Se establecen las herramientas disponibles para el usuario
//Determinan que algoritmo de dibujo se va a ejecutar
enum herramienta { Ninguna, Linea, Circulo,Poligono,balde,Cuadrado,Triangulo};
herramienta herramientaActual = Ninguna; //Inicia seleccionando la linea por defecto

//Esta seccion funciona en base a una maquina de estados
//0 -> espera el primer click
//1 -> espera el segundo click
// 
//Linea: el click 1 = inicio, click 2 = fin
//Circulo: el click  1 = centro, click 2 = radio
int estadoClick = 0;

//Coordenadas de celdas seleccionadas por el usuario
//Se usan como parametro en bresenham
int clickX0 = 0, clickY0 = 0; 
int clickX1 = 0, clickY1 = 0; 

//Color seleccionado desde ImGui
float colorFigura[4] = { 1.0,0.0f,0.0f,1.0f };//Rojo,Verde,azul,Alpha

float trasladoX = 0.0f;
float trasladoY = 0.0f;

float escalaFigura = 1.0f;

float rotacionFigura = 0.0f;

// Indice de la figura seleccionada
int figuraSeleccionada = 0;

//Variable del balde
int idRellenoActual = 2;

//Se implementa la matriz de cuadricula 
const int GRID_SIZE = 400; //0 = vacio , 1= borde, 2= relleno 
int grid[GRID_SIZE][GRID_SIZE] = { 0 };

float escalaGrid = 0.1f;
bool actualizarBuffers = false;

//Almacena la informacion necesaria para volver a dibujar
//Una figura en cada frame sin recalcular sus vertices

struct FiguraDibujada { //struct agrupa las variables en distintos tipos de datos bajo un mismo nombre
	
	GLuint vao = 0;
	GLuint vbo = 0;

	GLsizei conteoVertices = 0;
	GLenum modoDibujo = GL_TRIANGLES;

	//Se guardan los colores en cada figura
	float r = 1.0f;
	float g = 1.0f;
	float b = 1.0f;
	float a = 1.0f;

	//Transformaciones de figuras 
	float tx = 0.0f; //Traslacion en X
	float ty = 0.0f; //Traslacion en Y

	float escala = 1.0f;

	float rotacion = 0.0f;

	float centroX = 0.0f;
	float centroY = 0.0f;

};
struct Punto
{
	int x;
	int y;
};

std::vector<Punto> verticesPoligono;

//Guarda todas las figuras creadas para que permanezcan visibles en todo el programa
std::vector<Punto> ultimoRelleno;
std::vector<FiguraDibujada> historialFiguras;
std::vector<float> lineaVertices;
std::vector<float> circuloVertices;

GLuint VAO, VBO;

//Declaracion de funciones
std::vector<float> generarLineasBresenham(int x0, int y0, int x1, int y1, float escala);
std::vector<float>generarCirculoBresenham(int cx, int cy, int r, float escala);

//Declaracion de la funcion floodFill(rellena por inundacion o balde)

//Funcion de la herramienta Cuadrado
std::vector<float> generarCuadrado(
	int x0,
	int y0,
	int x1,
	int y1,
	float escala)
{
	std::vector<float> vertices;

	int lado =std::max(abs(x1 - x0),abs(y1 - y0));

	x1 =x0 +((x1 > x0)	? lado: -lado);

	y1 =y0 +((y1 > y0)? lado: -lado);

	auto lado1 =generarLineasBresenham(x0, y0,x1, y0,escala);

	auto lado2 =generarLineasBresenham(x1, y0,x1, y1,escala);

	auto lado3 =generarLineasBresenham(x1, y1,x0, y1,escala);

	auto lado4 =generarLineasBresenham(x0, y1,x0, y0,escala);

	vertices.insert(vertices.end(),
		lado1.begin(), lado1.end());

	vertices.insert(vertices.end(),
		lado2.begin(), lado2.end());

	vertices.insert(vertices.end(),
		lado3.begin(), lado3.end());

	vertices.insert(vertices.end(),
		lado4.begin(), lado4.end());

	return vertices;
}

//Funcion de la herramienta Triangulo
std::vector<float> generarTriangulo(int x0,int y0,int x1,int y1,float escala)
{
	std::vector<float> vertices;

	int lado = abs(x1 - x0);
	//Correcion del triangulo equilatero

	int cx = (x0 + x1) / 2;

	int altura =static_cast<int>(lado * 0.866f); // √3/2

	auto lado1 =generarLineasBresenham(	x0,y0,x1,y0,escala);

	auto lado2 =generarLineasBresenham(x1,y0,cx,y0 + altura,escala);

	auto lado3 =generarLineasBresenham(cx,y0 + altura,x0,y0,escala);

	vertices.insert(vertices.end(),
		lado1.begin(), lado1.end());

	vertices.insert(vertices.end(),
		lado2.begin(), lado2.end());

	vertices.insert(vertices.end(),
		lado3.begin(), lado3.end());

	return vertices;
}

//Funcion que genera los vertices relleno 
std::vector<float> generarVerticesRelleno(
	const std::vector<Punto>& celdas,
	float escala)
{
	std::vector<float> vertices;

	for (const auto& p : celdas)
	{
		int gx = p.x - GRID_SIZE / 2;
		int gy = p.y - GRID_SIZE / 2;

		float xMin = gx * escala;
		float xMax = (gx + 1) * escala;

		float yMin = gy * escala;
		float yMax = (gy + 1) * escala;

		vertices.insert(vertices.end(),
			{
				xMin,yMin,0.0f,
				xMin,yMax,0.0f,
				xMax,yMin,0.0f,

				xMin,yMax,0.0f,
				xMax,yMax,0.0f,
				xMax,yMin,0.0f
			});
	}

	return vertices;
}


//Funcion que comprueba el cierre del poligono
bool poligonoCerrado()
{
	if (verticesPoligono.size() < 3)
		return false;

	Punto primero = verticesPoligono.front();
	Punto ultimo = verticesPoligono.back();

	return (primero.x == ultimo.x &&
		primero.y == ultimo.y);
}
//Implementacion de Flood Fill
std::vector<Punto> floodFill(int startX, int startY)
{
	std::vector<Punto> celdasRellenadas;

	std::queue<Punto> cola;

	cola.push({ startX,startY });

	while (!cola.empty())
	{
		Punto p = cola.front();
		cola.pop();

		if (p.x < 0 || p.x >= GRID_SIZE)
			continue;

		if (p.y < 0 || p.y >= GRID_SIZE)
			continue;

		if (grid[p.x][p.y] != 0)
			continue;

		grid[p.x][p.y] = idRellenoActual;

		celdasRellenadas.push_back(p);

		cola.push({ p.x + 1,p.y });
		cola.push({ p.x - 1,p.y });
		cola.push({ p.x,p.y + 1 });
		cola.push({ p.x,p.y - 1 });
	}
	idRellenoActual++;

	return celdasRellenadas;
}


//GLFW ejecuta esta funcion automaticamente cuando la ventana cambia
// de tamaño, el viewport se adapta para seguir ocupando la ventana
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}

// Se gestionan los cambios de herramienta con el teclado
// L actica el modo linea, C activa el modo circulo
//Reinicia el estado de clicks para evitar inconsistencias

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

//Gestiona los click del raton, conviriendo la posicion del cursor a coordenadas discretas
// De la cuadricula utilizada por algoritmos de dibujo
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
	ImGui_ImplGlfw_MouseButtonCallback(window, button, action, mods);

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

		// 1. Conversión de coordenadas en píxeles al sistema NDC(normalized device coordinates o espacio de coordenadas 3D) de OpenGL
		//NDC va desde -1 a 1

		float ndcX = (2.0f * mouseX) / width - 1.0f;
		float ndcY = 1.0f - (2.0f * mouseY) / height;

		// 2. Conversión de NDC a coordenadas del mundo.
		//Permite determinar en qué posición lógica de la escena
		//se encuentra el cursor.
		float aspectRatio = (float)width / (float)height;
		float orthoHeight = 1.0f;
		float orthoWidth = orthoHeight * aspectRatio;

		float worldX = ndcX * orthoWidth;
		float worldY = ndcY * orthoHeight;

		// 3. Conversión de coordenadas continuas a coordenadas discretas.
		//Cada valor entero representa una celda de la cuadrícula.
		int gridX = std::floor(worldX / escalaGrid);
		int gridY = std::floor(worldY / escalaGrid);

		//Implementacion de la herramienta balde
		if (herramientaActual == balde)
		{
			ultimoRelleno =
				floodFill(
					gridX + GRID_SIZE / 2,
					gridY + GRID_SIZE / 2);

			actualizarBuffers = true;

			return;
		}

		//Sistema de dos clicks
		// primer click: almacena punto inicial
		// Segundo click: completa la figura y activa la generacion de vertices
				// ... (Código de conversión de coordenadas X y Y del mouse) ...

		if (estadoClick == 0) {
			clickX0 = gridX;
			clickY0 = gridY;
			estadoClick = 1;
			std::cout << "Primer Click en: (" << clickX0 << ", " << clickY0 << ")" << std::endl;
		}
		else if (estadoClick == 1)
		 {
			clickX1 = gridX;
			clickY1 = gridY;

			//Funcion que guarda los vertices
			if (herramientaActual == Poligono)
			{
				if (verticesPoligono.empty())
				{
					verticesPoligono.push_back(
						{
							clickX0,
							clickY0
						});
				}

				verticesPoligono.push_back(

					{
						clickX1,
						clickY1
					});
				//Deteccion del cierre del poligono
				if (poligonoCerrado())
				{
					std::cout
						<< "Poligono cerrado"
						<< std::endl;
				}
				if (poligonoCerrado())
				{
					int lados = verticesPoligono.size() - 1;

					if (lados == 3)
						std::cout << "TRIANGULO" << std::endl;

					else if (lados == 4)
						std::cout << "CUADRADO O RECTANGULO" << std::endl;
				}

			}
			//Dibujo de varias aristas consecutivas
			if (herramientaActual == Poligono)
			{
				clickX0 = clickX1;
				clickY0 = clickY1;

				estadoClick = 1;// Seguir esperando nuevos vertices
			}
			else
			{
				estadoClick = 0; //Lineas y circulos terminan aqui
			}

			// OBLIGATORIO: Forzar a que se active el buffer al hacer el segundo click
			actualizarBuffers = true;
			std::cout << "Segundo Click en: (" << clickX1 << ", " << clickY1 << ")" << std::endl;
		}

	}
}



//Vertex Shader Source Code
//Recibe los vertices y les aplica la matriz de proyeccion
//antes de enviarlos al pipeline grafico
const char* vertexShaderSource = "#version 330 core\n"
"layout (location = 0) in vec3 aPos;\n"
"uniform mat4 projection;\n"
"uniform mat4 model;\n" //Se modifican los vertex shader para aplicar transformaciones
"void main()\n"
"{\n"
" gl_Position = projection * model * vec4(aPos, 1.0);\n"
"}\0";

//Fragment Shader Source Code
//Define el color de los fragmentos generados
const char* fragmentShaderSource = "#version 330 core\n"
"out vec4 FragColor;\n"
"uniform vec4 objectColor;\n" // <- CORREGIDO: Todo en minúsculas para evitar conflictos
"void main()\n"
"{\n"
" FragColor = objectColor;\n"
"}\n\0";

// Funcion de Bresenham Modificada para alinear los cuadrados dentro de la grilla
// 
//Cada celda recorrida por la linea se representa mediante un
// cuadrado compuesto por dos triangulos

std::vector<float> generarLineasBresenham(int x0, int y0, int x1, int y1, float escala) {
	std::vector<float> vertices;
	int dx = abs(x1 - x0), dy = abs(y1 - y0);
	int sx = (x0 < x1) ? 1 : -1, sy = (y0 < y1) ? 1 : -1;
	int err = dx - dy;

	while (true) {
		// Implementacion del marcado de bordes de figuras 
		grid[x0 + GRID_SIZE / 2][y0 + GRID_SIZE / 2] = 1;
	
		// cada coordenada representa una celda de la cuadricula
		// se calculan los limites reales del cuadrado que ocupara esa celda
		float xMin = x0 * escala;
		float xMax = (x0 + 1) * escala;
		float yMin = y0 * escala;
		float yMax = (y0 + 1) * escala;

		// Triángulo 1 forma la mitad inferior del cuadrado
		vertices.push_back(xMin); vertices.push_back(yMin); vertices.push_back(0.0f);
		vertices.push_back(xMin); vertices.push_back(yMax); vertices.push_back(0.0f);
		vertices.push_back(xMax); vertices.push_back(yMin); vertices.push_back(0.0f);

		// Triángulo 2 completa la celda cuadrada
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
//Funcion de Bresenham para circulo de celdas rellenas
// Utiliza simetria de ocho octantes para minimizar calculos 
std::vector<float> generarCirculoBresenham(int cx, int cy, int r, float escala) {
	std::vector<float> vertices;

	int x = 0;
	int y = r;
	int d = 3 - 2 * r;

	//Funcion auxiliar que genera los vertices de la celda
	// evitando la repeticion de codigo cada vez que necesita dibujar un punto
	auto agregarCuadrado = [&](int xc, int yc) {
		//Marcar bordes de la figura
		grid[xc + GRID_SIZE / 2][yc + GRID_SIZE / 2] = 1;

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
	//Mientras no se supere la diagonal principal
	//se generan los ocho puntos equivalente del circulo

	while (y >= x) {

		// simetria de 8 vias
		//Reflejo del punto calculado de los ocho octantes del circulo

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


//*****************Funcion principal*****************
//Punto de entrada del programa 
// el cual inicializa OpenGL,GLFW. ImGui y ejecuta el bucle principal
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

	//Iniciar backends para glfw y openGL para traducir las ordenes graficas
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
	//Compilacion y enlazado de shaders
	//Estos programas se ejecutan directamente en la GPU

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
	//Generacion de vertices en la cuadricula
	//Se crea una red de lineas horizontales y verticales para el area de dibujo

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
	
		glGenVertexArrays(1, &VAO);
		glGenBuffers(1, &VBO);

		glBindVertexArray(VAO);

		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		// Pasa los vértices generados a la tarjeta gráfica
		glBufferData(GL_ARRAY_BUFFER, gridVertices.size() * sizeof(float), gridVertices.data(), GL_STATIC_DRAW);

		// Indicarle a OpenGL cómo leer los atributos (X, Y, Z)
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);

		// Desvincular de manera segura
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
	}



	// BUCLE DE RENDERIZADO 
    // Bucle principal de la aplicacion que se ejecuta continuamente
	// hasta que el usuario cierre la ventana
	   
	while (!glfwWindowShouldClose(window))
	{
		glfwPollEvents();

		//Arregla errores en transformaciones
		static int figuraAnterior = -1;

		// 1. INICIAR EL CONTEXTO DE IMGUI INMEDIATAMENTE 
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		// 2. DISEÑAR TU VENTANA FLOTANTE DE IMGUI

		ImGui::Begin("Herramientas del Paint");

		if (ImGui::Button("Modo Linea")) {
			herramientaActual = Linea; 
			estadoClick = 0;           // Reinicia el flujo de clics
			lineaVertices.clear();     // Borra el trazo anterior en pantalla si deseas limpiar
			circuloVertices.clear();   // Borra el círculo para dar espacio a la nueva línea
		}

		if (ImGui::Button("Modo Circulo"))
		{
			herramientaActual = Circulo;
			estadoClick = 0;             // Reinicia el flujo de clics
			lineaVertices.clear();       // Borra la línea anterior
			circuloVertices.clear();     // Borra el trazo anterior
		}

		if (ImGui::Button("Modo Poligono"))
			{
				herramientaActual = Poligono;
				estadoClick = 0;
				verticesPoligono.clear();
			}

		//Implementacion del boton balde
		if (ImGui::Button("Modo Balde"))
		{
			herramientaActual = balde;
			estadoClick = 0;
			verticesPoligono.clear();
		}
		
		ImGui::Separator(); //Se encarga de controlar los colores

		if (!historialFiguras.empty() &&
			figuraSeleccionada >= historialFiguras.size())
		{
			figuraSeleccionada =
				(int)historialFiguras.size() - 1;
		}

		//Boton de limpiar lienzo
		if (ImGui::Button("Limpiar Lienzo"))
		{
			historialFiguras.clear();

			verticesPoligono.clear();

			estadoClick = 0;

			memset(grid, 0, sizeof(grid));

			idRellenoActual = 2;
		}
		//Implementacion de cuadrado
		if (ImGui::Button("Modo Cuadrado"))
		{
			herramientaActual = Cuadrado;
			estadoClick = 0;
			verticesPoligono.clear();
		}
		//Implementacion de triangulo
		if (ImGui::Button("Modo Triangulo"))
		{
			herramientaActual = Triangulo;
			estadoClick = 0;
			verticesPoligono.clear();
		}

		//Slider modifica las figuras en tiempo real
		if (!historialFiguras.empty())
		{
			ImGui::SliderInt("Figura",&figuraSeleccionada,0,(int)historialFiguras.size() - 1);
		}

		//Se añaden los controles de transformaciones de figuras
		ImGui::Text("Transformaciones");

		ImGui::SliderFloat(	"Mover X",	&trasladoX,	-1.0f,1.0f);

		ImGui::SliderFloat("Mover Y",&trasladoY,-1.0f,1.0f);

		ImGui::SliderFloat("Escala",&escalaFigura,0.1f,5.0f);

		ImGui::SliderFloat("Rotacion",&rotacionFigura,0.0f,360.0f);

		if (!historialFiguras.empty())
		{
			if (figuraSeleccionada != figuraAnterior)
			{
				trasladoX =
					historialFiguras[figuraSeleccionada].tx;

				trasladoY =
					historialFiguras[figuraSeleccionada].ty;

				escalaFigura =
					historialFiguras[figuraSeleccionada].escala;

				rotacionFigura =
					historialFiguras[figuraSeleccionada].rotacion;

				figuraAnterior =
					figuraSeleccionada;
			}

			historialFiguras[figuraSeleccionada].tx =
				trasladoX;

			historialFiguras[figuraSeleccionada].ty =
				trasladoY;

			historialFiguras[figuraSeleccionada].escala =
				escalaFigura;

			historialFiguras[figuraSeleccionada].rotacion =
				rotacionFigura;
		}


		ImGui::Text("Color de la figura");

		ImGui::ColorEdit4("##ColorFigura",colorFigura);

		//Nombre herramienta en menu
		const char* nombreHerramienta = "NINGUNA";

		switch (herramientaActual)
		{
		case Linea:
			nombreHerramienta = "LINEA";
			break;

		case Circulo:
			nombreHerramienta = "CIRCULO";
			break;

		case Poligono:
			nombreHerramienta = "POLIGONO";
			break;
		case balde:
			nombreHerramienta = "BALDE";
			break;
		case Cuadrado:
			nombreHerramienta = "CUADRADO";
			break;
		case Triangulo:
			nombreHerramienta = "TRIANGULO";
			break;
		}

		// Mostrar dinámicamente la herramienta activa en el menú
		ImGui::Text("Herramienta activa: %s", nombreHerramienta);

		ImGui::End();


		// 3. LIMPIAR PANTALLA
		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		// 4. ACTUALIZACIÓN DE BUFFERS DE BRESENHAM 
		// Si se completa la figura por medio de dos clicks
		// Se generan y se envian los vertices a la GPU

		if (actualizarBuffers) {
			GLuint nuevaVAO, nuevaVBO;
			glGenVertexArrays(1, &nuevaVAO);
			glGenBuffers(1, &nuevaVBO);
			glBindVertexArray(nuevaVAO);
			glBindBuffer(GL_ARRAY_BUFFER, nuevaVBO);

			FiguraDibujada nuevaFigura;
			nuevaFigura.tx = 0.0f;
			nuevaFigura.ty = 0.0f;

			nuevaFigura.escala = 1.0f;

			nuevaFigura.rotacion = 0.0f;

			nuevaFigura.vao = nuevaVAO;
			nuevaFigura.vbo = nuevaVBO;
			nuevaFigura.modoDibujo = GL_TRIANGLES;
			
			//Crear una figura de relleno
			if (herramientaActual == balde)
			{

				auto rellenoVertices =
				generarVerticesRelleno(	ultimoRelleno,	escalaGrid);

				glBufferData(
					GL_ARRAY_BUFFER,
					rellenoVertices.size() * sizeof(float),
					rellenoVertices.data(),
					GL_STATIC_DRAW);

				nuevaFigura.conteoVertices =
					rellenoVertices.size() / 3;

				nuevaFigura.r = colorFigura[0];
				nuevaFigura.g = colorFigura[1];
				nuevaFigura.b = colorFigura[2];
				nuevaFigura.a = colorFigura[3];

				//Guardar el centro al crear una figura 
				nuevaFigura.centroX =
					(clickX0 + clickX1) * escalaGrid * 0.5f;

				nuevaFigura.centroY =
					(clickY0 + clickY1) * escalaGrid * 0.5f;

				historialFiguras.push_back(nuevaFigura);

			}

			//Figura linea 
			if (herramientaActual == Linea) {
				lineaVertices = generarLineasBresenham(clickX0, clickY0, clickX1, clickY1, escalaGrid);
				glBufferData(GL_ARRAY_BUFFER, lineaVertices.size() * sizeof(float), lineaVertices.data(), GL_STATIC_DRAW);
				nuevaFigura.conteoVertices = lineaVertices.size() / 3;

				//Guarda el centro al crear una figura 
				nuevaFigura.centroX =
					(clickX0 + clickX1) * escalaGrid * 0.5f;

				nuevaFigura.centroY =
					(clickY0 + clickY1) * escalaGrid * 0.5f;

				//Se configuran los colores 
				nuevaFigura.r = colorFigura[0];
				nuevaFigura.g = colorFigura[1];
				nuevaFigura.b = colorFigura[2];
				nuevaFigura.a = colorFigura[3];

				historialFiguras.push_back(nuevaFigura); // Guarda la línea en el lienzo
			//Permite que la figura siga dibujandose en los frames posteriores	
			}
			else if (herramientaActual == Circulo) {
				int radio = std::round(std::sqrt(std::pow(clickX1 - clickX0, 2) + std::pow(clickY1 - clickY0, 2)));
				circuloVertices = generarCirculoBresenham(clickX0, clickY0, radio, escalaGrid);
				glBufferData(GL_ARRAY_BUFFER, circuloVertices.size() * sizeof(float), circuloVertices.data(), GL_STATIC_DRAW);
				nuevaFigura.conteoVertices = circuloVertices.size() / 3;

				nuevaFigura.r = colorFigura[0];
				nuevaFigura.g = colorFigura[1];
				nuevaFigura.b = colorFigura[2];
				nuevaFigura.a = colorFigura[3];

				//Guardar el centro al crear una figura 
				nuevaFigura.centroX =
					(clickX0 + clickX1) * escalaGrid * 0.5f;

				nuevaFigura.centroY =
					(clickY0 + clickY1) * escalaGrid * 0.5f;

				historialFiguras.push_back(nuevaFigura); // Guarda el círculo en el lienzo
			}
			//Generacion de lineas automaticamente
			else if (herramientaActual == Poligono)
			{
				lineaVertices =
					generarLineasBresenham(
						clickX0,
						clickY0,
						clickX1,
						clickY1,
						escalaGrid);

				glBufferData(
					GL_ARRAY_BUFFER,
					lineaVertices.size() * sizeof(float),
					lineaVertices.data(),
					GL_STATIC_DRAW);

				nuevaFigura.conteoVertices =
					lineaVertices.size() / 3;

				nuevaFigura.r = colorFigura[0];
				nuevaFigura.g = colorFigura[1];
				nuevaFigura.b = colorFigura[2];
				nuevaFigura.a = colorFigura[3];

				historialFiguras.push_back(nuevaFigura);
			}
			//Herramienta de cuadrado
			else if (herramientaActual == Cuadrado)
			{
				auto vertices =
					generarCuadrado(
						clickX0,
						clickY0,
						clickX1,
						clickY1,
						escalaGrid);

				glBufferData(
					GL_ARRAY_BUFFER,
					vertices.size() * sizeof(float),
					vertices.data(),
					GL_STATIC_DRAW);

				nuevaFigura.conteoVertices =
					vertices.size() / 3;

				nuevaFigura.r = colorFigura[0];
				nuevaFigura.g = colorFigura[1];
				nuevaFigura.b = colorFigura[2];
				nuevaFigura.a = colorFigura[3];

				//Guardar el centro al crear una figura 
				nuevaFigura.centroX =
					(clickX0 + clickX1) * escalaGrid * 0.5f;

				nuevaFigura.centroY =
					(clickY0 + clickY1) * escalaGrid * 0.5f;

				historialFiguras.push_back(nuevaFigura);

			}
			//Herramienta de triangulo
			else if (herramientaActual == Triangulo)
			{
				auto vertices =
					generarTriangulo(
						clickX0,
						clickY0,
						clickX1,
						clickY1,
						escalaGrid);

				glBufferData(
					GL_ARRAY_BUFFER,
					vertices.size() * sizeof(float),
					vertices.data(),
					GL_STATIC_DRAW);

				nuevaFigura.conteoVertices =
					vertices.size() / 3;

				nuevaFigura.r = colorFigura[0];
				nuevaFigura.g = colorFigura[1];
				nuevaFigura.b = colorFigura[2];
				nuevaFigura.a = colorFigura[3];

				//Guardar el centro al crear una figura 
				nuevaFigura.centroX =
					(clickX0 + clickX1) * escalaGrid * 0.5f;

				nuevaFigura.centroY =
					(clickY0 + clickY1) * escalaGrid * 0.5f;


				historialFiguras.push_back(nuevaFigura);
			}

			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
			glEnableVertexAttribArray(0);
			glBindBuffer(GL_ARRAY_BUFFER, 0);
			glBindVertexArray(0);

			actualizarBuffers = false;
		}

		// 5. OBTENER TAMAÑO Y ENVIAR MATRIZ PROJECTION
		// Construye la proyeccion ortografica
		// Mantiene la escala de la cuadricula independientemente del tamaño de la ventana
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

		glUniformMatrix4fv(projloc,	1,GL_FALSE,projection); //Matriz de proyeccion

		GLuint modelLoc =glGetUniformLocation(shaderProgram,"model"); // Obtener la ubicacion del uniform

		GLuint colorLoc = glGetUniformLocation(shaderProgram, "objectColor");

		// 6. DIBUJAR LA GRILLA BLANCA
		glUniform4f(colorLoc, 1.0f, 1.0f, 1.0f, 1.0f);
		//Evita que se deforme la grilla 
		glBindVertexArray(VAO);

		float identidad[16] =
		{
			1,0,0,0,
			0,1,0,0,
			0,0,1,0,
			0,0,0,1
		};

		glUniformMatrix4fv(
			modelLoc,
			1,
			GL_FALSE,
			identidad);
		glDrawArrays(GL_LINES, 0, gridVertices.size() / 3);

		// Redibuja las figuras almacenadas en el lienzo
		for (const auto& fig : historialFiguras){
			float angulo =
				fig.rotacion *
				3.1415926f / 180.0f;

			float c = cos(angulo);
			float s = sin(angulo);

			//Model matrix
			float tx = fig.tx;
			float ty = fig.ty;

			float cx = fig.centroX;
			float cy = fig.centroY;

			float sx = fig.escala;
			float sy = fig.escala;

			float model[16] =
			{
				sx * c,
				sx * s,
				0.0f,
				0.0f,

			   -sy * s,
				sy * c,
				0.0f,
				0.0f,

				0.0f,
				0.0f,
				1.0f,
				0.0f,

				cx + tx - (sx * c * cx - sy * s * cy),
				cy + ty - (sx * s * cx + sy * c * cy),
				0.0f,
				1.0f
			};
			//Usar los colores al dibujar una figura

			glUniform4f(colorLoc,fig.r,fig.g,fig.b,	1.0f);
			glBindVertexArray(fig.vao);

			//Enviar Matrix Model al shader 
			glUniformMatrix4fv(modelLoc,1,GL_FALSE,model);
			glDrawArrays(fig.modoDibujo, 0, fig.conteoVertices);
		}
		glBindVertexArray(0);

		// 9. ENVIAR LOS DATOS DE IMGUI A DIBUJAR (Al final de todo el pipeline gráfico)
		// Renderiza la interfaz grafica(botones, textos, ventanas)
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		// 10. REFRESCAR MONITOR
		//Intercambia el buffer trasero y delantero
		//Muestra en la pantalla los dibujos
		// 
		glfwSwapBuffers(window);
	}

	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);

	//Liberar los VAO Y VBO
	for (auto& fig : historialFiguras)
	{
		glDeleteVertexArrays(1,&fig.vao);

		glDeleteBuffers(1,&fig.vbo);
	}

	//Limpieza
	glDeleteVertexArrays(1, &VAO); glDeleteBuffers(1, &VBO);
	glDeleteProgram(shaderProgram);
	glfwDestroyWindow(window);
	glfwTerminate();
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	return 0;
}
