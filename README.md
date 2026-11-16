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
    ├── User input
    │   ├── Mouse
    │   ├── keyboard
    │   └── ImGui
    │
    ├── State Machines
    │   ├── First Click
    │   └── Second Click
    │
    ├── Drawing Algorithms
    │   ├── Bresenham Line
    │   ├── Bresenham Círcle
    │   ├── Square
    │   ├── Triangle
    │   └── Flood Fill
    │
    ├── Figure Management
    │   ├── VAO
    │   ├── VBO
    │   └── History
    │
    └── OpenGL Rendering
     ├── Vertex Shader  
     ├── Fragment Shader
     └── ImGui Interface

**Used Technologies**
===============
| Technology | Use | 
|--------|------|
| **`C++`** | Principal Logic |
| **`OpenGL 3.3`** | Render |
| **`GLFW`** | Window and events |
| **`GLAD`** | Loading OpenGL functions |
| **`ImGui`** | Graphical interface  |
| **`STL`** | Vectors and queues |

**Available Tools**
===============
| Tool | Description | 
|--------|------|
| **`Line`** | Bresenham Algorithms |
| **`Circle`** | Bresenham Algorithms |
| **`Polygon`** | Segment Draw |
| **`Square`** | Square Based in Bresenham |
| **`Triangle`** | Equilateral triangles |
| **`Bucket`** | Flood fill |
| **`Transformations`** | Rotation, Scale, Translation |

**Branch Map**
===============
    main
    │
    ├── feature-bresenham-line
    │
    ├── feature-bresenham-circle
    │
    ├── feature-polygons
    │
    ├── feature-squares
    │
    ├── feature-triangles
    │
    ├── feature-floodfill
    │
    ├── feature-imgui
    │
    ├── feature-transformations
    │
    └── final-release

**Branch Roles**
===============
| Branch | Role | Focus | Content | 
|--------|------|--------|------|
| **`Main`** | Production  | stable code  | final version completely functional | 
| **`feature-bresenham-line`** | Development  | Raster | Implementation of the line algorithm | 
| **`feature-bresenham-circle** | Development | Raster | Implementation of the circle | 
| **`feature-polygons`** | Development | Geometry | Polygons construction | 
| **`feature-squares`** | Development | Figures | Squares | 
| **`feature-triangles`** | Development |  Figures| Equilateral triangles | 
| **`feature-floodfill`** | Development | Filling | Bucket tool | 
| **`feature-imgui`** | Development | UI | tools panel |
| **`feature-transformations`** | Development | Transformations | Rotation,Scale,Translation |
| **`final-release`** | Delivery | Consolidation | Delivery of the final exam  |

**Suggested Learning Path**
===============
1. **`main()`** -GLFW + GLAD + Shaders = OpenGl initialization
2. **`mouse_button_callback()`** Mouse -> NDC (Normalized device coordinates-> World Space -> = Coordinates System  
