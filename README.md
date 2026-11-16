**OPENGL PROJECT - COMPUTER GRAPHICS - PAINT**
===============
### Name: Juan Diego Gutierrez Dueñas
### Student Code: 6014463
### Institution: Universidad Militar Nueva Granada
**DESCRIPTION OF THE PROJECT**
===============
App based on paint, developed in C++ using OpenGL,GLFW,GLAD and ImGui.

The system let you draw graphic primitives over a grid using raster algorithms

The created figures stay storaged in the memory and they can be manipulated using

geometric transformations.

Remote:https://github.com/JuanDiegoGutierrezDuenas/GridOpenGL

**Implemented funcionalities**
===============
- Line using Bresenham
- Circle using Bresenham
- Polygons
- Squares
- Equilateral triangles
- Flood Fill(bucket)
- Color Selection
- Translation
- Scaling
- Rotation
- History of figures
- Canvas Cleaning
  
**Proyect Architecture**
===============
Paint OpenGL

│

├── Entrada de usuario

│   ├── Mouse

│   ├── Teclado
│   └── ImGui
│
├── Máquina de Estados

│   ├── Primer Click

│   └── Segundo Click

│

├── Algoritmos de Dibujo

│   ├── Bresenham Línea

│   ├── Bresenham Círculo

│   ├── Cuadrado

│   ├── Triángulo

│   └── Flood Fill

│

├── Gestión de Figuras

│   ├── VAO

│   ├── VBO

│   └── Historial

│

└── Renderizado OpenGL

│  ├── Vertex Shader
    
│ ├── Fragment Shader
    
│ └── ImGui Interface
