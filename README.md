# M.I.C.R.O Graphics Engine (OpenGL)

 <br>
 
![M I C R O Graphics Engine](https://github.com/user-attachments/assets/4d229ca2-4446-48bf-b933-e0b2bfa5a8da)

 <br>

---

## Project Description

The **M.I.C.R.O Graphics Engine**, or "My Input Controlled Real-Time Output Graphics Engine," is a custom-built graphics engine developed using OpenGL. It allows real-time manipulation of 3D models and their properties, such as position, rotation, scale, lighting, and textures. The project features a UI built with Dear ImGUI for controlling these parameters interactively, alongside real-time lighting effects. This engine serves as a foundational graphics framework for learning and experimentation, offering a hands-on introduction to core rendering techniques and real-time 3D graphics.

---

## Features

- **Model Loading**: Loads 3D models (**.obj** format) and allows real-time manipulation of their properties.
- **Lighting System**: Enables dynamic lighting, affecting object shading and appearance.
- **Texture Control**: Activate or deactivate textures on the 3D models.
- **UI Controls**: Dear ImGUI interface to modify object transformations (position, rotation, scale) and adjust lighting and texture properties.
- **Model Loader**: A dedicated button in the UI allows users to load new models dynamically into the scene.

---

<br>

# Loading a New Model

To load a new model dynamically into the scene, follow these steps:

1. From the Project's Root, browse to **`Release -> Platforms -> (x64 or x86) -> OGL-P2 -> Assets `**. Inside this folder you will find the following folders:

   - Models
   - Materials
   - Textures

2. Place the necessary files in the correct directories inside Assets:

   - .**obj** files in **Assets/Models/**

   - **.mtl** material files in **Assets/Materials/**

   - **.jpg** or **.png** texture files in **Assets/Textures/**

3. Once inside the program (Check [Installation & Running](https://github.com/Pecas-Dev/M.I.C.R.O-Graphics-Engine?tab=readme-ov-file#installation--running)), click on the **_Load New Model button._**

4. Browse to **`Release -> Platforms -> (x64 or x86) -> OGL-P2 -> Assets-> Models`** and select your **.obj** file.

5. The engine will automatically attempt to load the associated material and textures.

<br>

---

## Shaders

### Vertex Shader (Default)

Handles the transformation of 3D coordinates and passes color data for rendering.

```glsl
in vec3 vertexIn;
in vec4 colorIn;
out vec4 colorOut;

uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;

void main()
{
    colorOut = colorIn;
    gl_Position = proj * view * model * vec4(vertexIn, 1.0);
}
```

### Fragment Shader (Default)

Renders color and lighting information onto the object.

```glsl
in vec4 colorOut;
out vec4 fragColor;

void main()
{
    fragColor = colorOut;
}

```

### Vertex Shader (Light)

Handles the transformation of 3D coordinates for the light shader and normal calculations.

```glsl
#version 460

in vec3 vertexIn;
in vec4 colorIn;
in vec2 textureIn;
in vec3 normalIn;

out vec3 vertexOut;
out vec4 colorOut;
out vec2 textureOut;
out vec3 normalOut;

uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;
uniform mat3 normal;

void main()
{
    colorOut = colorIn;
    textureOut = textureIn;
    normalOut = normalize(normal * normalIn);

    vertexOut = (model * vec4(vertexIn, 1.0)).xyz;

    gl_Position = proj * view * model * vec4(vertexIn, 1.0);
}
```

### Fragment Shader (Light)

Handles the dynamic lighting calculations and texture mapping.

```glsl
struct Light
{
    vec3 position;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

struct Material
{
    float shininess;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

in vec3 vertexOut;
in vec2 textureOut;
in vec3 normalOut;

out vec4 fragColor;

uniform Light light;
uniform Material material;
uniform sampler2D textureImage;
uniform bool isTextured;

void main()
{
    vec3 ambientColor = light.ambient * material.ambient;
    vec3 lightDirection = normalize(light.position - vertexOut);
    vec3 normal = normalize(normalOut);
    float lightIntensity = max(dot(lightDirection, normal), 0.0);
    vec3 diffuseColor = light.diffuse * material.diffuse * lightIntensity;

    vec3 finalColor = ambientColor + diffuseColor;

    if(isTextured)
    {
        fragColor = vec4(finalColor, 1.0) * texture(textureImage, textureOut);
    }
    else
    {
        fragColor = vec4(finalColor, 1.0);
    }
}
```

---

## Directory Structure

```
Project Root
├── OpenGL2.sln                      // Visual Studio solution file
├── .gitattributes
├── .gitignore
├── bin                              // Binary files
├── Dependencies                     // External dependencies
│   ├── SDL
│   └── GLM
│
├── Release                          // Pre-compiled executables
│   ├── M.I.C.R.O x64.bat            // Run x64 M.I.C.R.O
│   ├── M.I.C.R.O x86.bat            // Run x86 M.I.C.R.O
│   └── Platforms
│       ├── x64                      // 64-bit builds
│       │   └── OGL-P2               // Contains the necessary files to run the executable.
│       │       └── Assets
│       │           ├── Models       // Store `.obj` model files HERE.
│       │           ├── Materials    // Stores `.mtl` material files HERE.
│       │           └── Textures     // Stores texture files (`.jpg` or `.png`) HERE.
│       │
│       │
│       │
│       └── Win32                    // 32-bit builds
│           └── OGL-P2               // Contains the necessary files to run the executable.
│               └── Assets
│                   ├── Models       // Store `.obj` model files HERE.
│                   ├── Materials    // Stores `.mtl` material files HERE.
│                   └── Textures     // Stores texture files (`.jpg` or `.png`) HERE.
│
└── OpenGL2
    ├── Assets                       // Stores shaders, models, textures, fonts, and materials.
    │   ├── Models
    │   ├── Materials
    │   ├── Textures
    │   └── Shaders
    │
    └── Project                      // Contains the source code for the M.I.C.R.O Graphics Engine.
        ├── include
        └── source
```

---

<br>

## Installation & Running

<br>

**Clone the repository:**

```bash
git clone https://github.com/Pecas-Dev/M.I.C.R.O-Graphics-Engine.git
```

### Pre-compiled Executables

The easiest way to run the simulation is using the pre-compiled executables:

1. Navigate to the `Release` folder.
2. Choose the appropriate version for your system (`x64` or `86`).
3. Run the `M.I.C.R.O x64.bat` or `M.I.C.R.O x86.bat`or file.

### Building from Source

To build the project from source:

1. Open `OpenGL2.sln` in Visual Studio.
2. Choose your configuration (**Debug/Release**) and platform (**x64/x86**).
3. Build the solution.
4. Run the program.

<br>

**Once the program is running:**

- **You can manipulate 3D models and adjust properties like position, scale, rotation, lighting, and textures using the Dear ImGUI UI.**

<br>

---

## Controls

- W/A/S/D: Move the light source across the scene.
- Use Dear ImGUI to adjust model properties, lighting, and textures in real time.

<br>

## Credits

This project was created by _**Pecas Dev**_.
