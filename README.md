# M.I.C.R.O Graphics Engine (OpenGL)

 <br>
 
![M I C R O Graphics Engine](https://github.com/user-attachments/assets/4d229ca2-4446-48bf-b933-e0b2bfa5a8da)

 <br>

--------------------------------
 
## Project Description

The **M.I.C.R.O Graphics Engine**, or "My Input Controlled Real-Time Output Graphics Engine," is a custom-built graphics engine developed using OpenGL. It allows real-time manipulation of 3D models and their properties, such as position, rotation, scale, lighting, and textures. The project features a UI built with Dear ImGUI for controlling these parameters interactively, alongside real-time lighting effects. This engine serves as a foundational graphics framework for learning and experimentation, offering a hands-on introduction to core rendering techniques and real-time 3D graphics.

--------------------------------
## Features

- **Model Loading**: Loads 3D models (.obj format) and allows real-time manipulation of their properties.
- **Lighting System**: Enables dynamic lighting, affecting object shading and appearance.
- **Texture Control**: Activate or deactivate textures on the 3D models.
- **UI Controls**: Dear ImGUI interface to modify object transformations (position, rotation, scale) and adjust lighting and texture properties.

--------------------------------

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


## Directory Structure
- **Dependencies**: External libraries like SDL and GLM required for rendering.
- **Release**: Contains the executable for running the graphics engine directly.
- **OpenGL2**: The main project folder containing:
    - **Assets**: Stores shaders, models, textures, fonts, and materials.
    - **Project**: Divided into source and include folders, it contains the source code for the graphics engine, organized into:
        - **Buffer, Camera, Grid, Light, Material, Shader, Texture**: Specialized folders for organizing core graphics components.
        - **UI, Utility**: Contains UI management and utility functions, like parsing and message handling.

## How to Run

1. Clone the repository:

```bash
git clone https://github.com/Pecas-Dev/M.I.C.R.O-Graphics-Engine.git
```
2. Navigate to the Release folder within the project directory. Inside, you will find an executable that allows you to run the app directly.

3. If you prefer to build the project manually, open the solution in Visual Studio, choose the x86 Platform, and build the project.


4. Once the program is running, you can manipulate 3D models and adjust properties like position, scale, rotation, lighting, and textures using the Dear ImGUI UI.

## Controls

- W/A/S/D: Move the light source across the scene.
- Use Dear ImGUI to adjust model properties, lighting, and textures in real time.

## Credits

This project was created by _**Pecas Dev**_.
