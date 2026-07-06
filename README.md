# M.I.C.R.O Graphics Engine (OpenGL)

 <br>
 
![M I C R O Graphics Engine](https://github.com/user-attachments/assets/4d229ca2-4446-48bf-b933-e0b2bfa5a8da)

 <br>

---

## Project Description

The **M.I.C.R.O Graphics Engine**, or "My Input Controlled Real-Time Output Graphics Engine," is a custom-built graphics engine developed using OpenGL. It allows real-time manipulation of 3D models and their properties, such as position, rotation, scale, lighting, and textures. The project supports a wide range of 3D model formats (**.obj**, **.fbx**, **.gltf**, **.glb**, **.dae**, **.stl**, **.ply**, **.3ds**) and features a physically based (PBR) rendering pipeline with normal mapping, image-based lighting from an HDR skybox, real-time shadow mapping, and an HDR post-processing pipeline (bloom + ACES tonemapping). Navigation uses optimized camera controls (RMB freelook + WASD fly, LMB orbit, MMB pan), and the UI is a custom-styled, dockable Dear ImGUI layout with a dedicated Scene viewport. This engine serves as a foundational graphics framework for learning and experimentation, offering a hands-on introduction to core rendering techniques and real-time 3D graphics.

 <br>

![InsideTheEngine](https://github.com/user-attachments/assets/f53976ce-569b-4aa1-b037-2ba52d86820b)

---

## Features

- **Multi-Format Model Loading**: Loads 3D models in a wide range of formats, **.obj**, **.fbx**, **.gltf**, **.glb**, **.dae**, **.stl**, **.ply**, and **.3ds**, and allows real-time manipulation of their properties. OBJ files use a custom, robust parser (supporting all face formats, quads/n-gons, negative indices, and missing normals), while all other formats are handled through the [Assimp](https://github.com/assimp/assimp) library.
- **PBR Rendering**: Physically based Cook-Torrance shading (GGX distribution, Smith geometry, Fresnel-Schlick) with **normal mapping**, **roughness maps**, and **metallic maps**, plus distance attenuation and gamma-correct, linear-space lighting.
- **Real-Time Shadows**: Shadow mapping with a dedicated depth pass, slope-scaled bias, and 3x3 PCF filtering for soft shadow edges.
- **Smart Asset Resolution**: Materials and textures are resolved relative to the model file, so models can be loaded from any folder on disk. Embedded textures (e.g. inside **.glb** files) are supported, and the engine scans for base color / normal / roughness / metallic maps by naming convention when a model doesn't reference them directly.
- **Auto-Framing**: Every model is automatically rescaled and centered on load, so assets authored in different units (meters, centimeters, inches) always appear at a sensible size, standing on the grid.
- **Image-Based Lighting & Skybox**: An HDR panorama is baked at startup into an environment cubemap, a diffuse irradiance map, a prefiltered specular map (multiple roughness mips), and a BRDF integration LUT, giving metallic materials real environment reflections in addition to the direct light.
- **HDR Post-Processing**: The scene renders in linear HDR (RGBA16F) and passes through a bright-pass threshold, separable gaussian bloom, and an ACES filmic tonemap with adjustable exposure before reaching the screen.
- **Dockable UI**: Built on Dear ImGUI's docking branch, the Scene, Properties, and Console panels can be freely rearranged, resized, and re-docked; the 3D scene renders into an offscreen target displayed inside the dockable Scene window.
- **Optimized Camera Controls**: Right-click to freelook and fly with WASD/QE (Shift to move faster), left-click drag to orbit the focus point, middle-click to pan, and scroll to dolly in and out.
- **Custom-Styled UI**: A modern Dear ImGUI theme (rounded corners, indigo accent, refined spacing) with controls for object transforms, lighting (position, color, intensity, ambient, falloff), environment/IBL, rendering (exposure, bloom), textures, and model loading.
- **Model Loader**: A dedicated button in the UI allows users to load new models dynamically into the scene.

 <br>

---

## Loading a New Model

The engine resolves each model's materials and textures **relative to the model file itself**, so you can load a model straight from any folder on your computer, no need to copy files into the engine's `Assets` folders first.

**Supported formats:** `.obj`, `.fbx`, `.gltf`, `.glb`, `.dae`, `.stl`, `.ply`, `.3ds`

To load a new model dynamically into the scene:

1. Once inside the program (Check [Installation & Running](https://github.com/Pecas-Dev/M.I.C.R.O-Graphics-Engine?tab=readme-ov-file#installation--running)), click on the **_Load New Model_** button.

2. Browse to your model file (in any supported format) anywhere on disk and select it.

3. The engine will automatically load the model along with its associated materials and textures. It looks for texture files next to the model, inside a `textures/` subfolder (a common convention on sites like Sketchfab), and in the engine's `Assets/Textures/` folder. Textures embedded directly inside the model file (e.g. `.glb`) are loaded automatically.

> **Tip:** Models are automatically rescaled and centered on load, so they always appear at a sensible size standing on the grid, the **Scale** slider is there for fine-tuning. Any warnings (missing textures, skipped data) are printed to the in-app **Console** panel.

 <br>

![Model](https://github.com/user-attachments/assets/82436d78-61b2-4b5c-9d21-e7d28df40907)

### Using the Assets Folders (optional)

You can still place assets in the engine's `Assets` folders if you prefer. Browse to the engine's install folder (**`Program Files\M.I.C.R.O\Assets`** if you used the installer, or **`Release\MICRO-win-(x64 or x86)\Assets`** for the portable folder), and place:

- model files in **Assets/Models/**
- **.mtl** material files in **Assets/Materials/**
- **.jpg** or **.png** texture files in **Assets/Textures/**

 <br>

---

## Shaders


### Vertex Shader (Default)

Handles the transformation of 3D coordinates and passes color data for rendering.

<details>

<summary>Vertex Shader (Default) Code</summary>

```glsl
#version 460

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
</details>

### Fragment Shader (Default)

Renders color and lighting information onto the object.

<details>

<summary>Fragment Shader (Deafult) Code</summary>

```glsl
#version 460

in vec4 colorOut;
out vec4 fragColor;

void main()
{
	fragColor = vec4(pow(colorOut.rgb, vec3(2.2)), colorOut.a);
}
```
</details>

### Vertex Shader (Light)

Handles the transformation of 3D coordinates for the lit shader, builds the tangent-space basis (TBN) used for normal mapping, and projects each vertex into the light's clip space for shadow mapping.

<details>

<summary>Vertex Shader (Light) Code</summary>

```glsl
#version 460

in vec3 vertexIn;
in vec4 colorIn;
in vec2 textureIn;
in vec3 normalIn;
in vec3 tangentIn;

out vec3 vertexOut;
out vec4 colorOut;
out vec2 textureOut;
out vec3 normalOut;
out mat3 TBN;
out vec4 lightSpacePosition;

uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;
uniform mat3 normal;
uniform mat4 lightSpaceMatrix;

void main()
{
    colorOut = colorIn;
    textureOut = textureIn;
    normalOut = normalize(normal * normalIn);

    // Tangent-space basis for normal mapping (Gram-Schmidt re-orthogonalized)
    vec3 T = normal * tangentIn;
    vec3 N = normalOut;
    T = T - dot(T, N) * N;
    TBN = mat3(normalize(T), cross(N, normalize(T)), N);

    vertexOut = (model * vec4(vertexIn, 1.0)).xyz;
    lightSpacePosition = lightSpaceMatrix * vec4(vertexOut, 1.0);

    gl_Position = proj * view * model * vec4(vertexIn, 1.0);
}
```
</details>

### Fragment Shader (Light)

Physically based (PBR) lighting using the **Cook-Torrance** specular BRDF, GGX normal distribution, Smith geometry with Schlick-GGX, and Fresnel-Schlick, driven per pixel by optional **normal**, **roughness**, and **metallic** maps. Shadows come from a shadow map sampled with slope-scaled bias and 3x3 PCF. All lighting happens in linear space: sRGB textures are linearized on input and the final color is gamma-corrected on output. (See `Assets/Shaders/Light.frag` for the full implementation, and `Depth.vert`/`Depth.frag` for the shadow map's depth-only pass.)

<details>

<summary>Fragment Shader (Light) Code</summary>

```glsl
// Core of the Cook-Torrance specular BRDF (excerpt)

// D: GGX normal distribution
float a2 = pow(roughness * roughness, 2.0);
float denominator = NdotH * NdotH * (a2 - 1.0) + 1.0;
float D = a2 / (PI * denominator * denominator);

// G: Smith geometry term with Schlick-GGX
float k = (roughness + 1.0) * (roughness + 1.0) / 8.0;
float G = (NdotV / (NdotV * (1.0 - k) + k)) * (NdotL / (NdotL * (1.0 - k) + k));

// F: Fresnel-Schlick; metals tint their reflections with the albedo
vec3 F0 = mix(vec3(0.04), albedo, metallic);
vec3 F = F0 + (1.0 - F0) * pow(1.0 - HdotV, 5.0);

vec3 specular = (D * G * F) / max(4.0 * NdotV * NdotL, 0.0001);

// Energy conservation: light that reflects doesn't also diffuse
vec3 kD = (vec3(1.0) - F) * (1.0 - metallic);

vec3 directLight = (kD * albedo / PI + specular) * radiance * NdotL * (1.0 - shadow);
```
</details>


### Other Shaders

<details>

<summary>Find The Other Shaders Here</summary>

 <br>

- [**BRDF.frag**](https://github.com/Pecas-Dev/M.I.C.R.O-Graphics-Engine/blob/main/OpenGL2/Assets/Shaders/BRDF.frag)
- [**BRDF.vert**](https://github.com/Pecas-Dev/M.I.C.R.O-Graphics-Engine/blob/main/OpenGL2/Assets/Shaders/BRDF.vert)
- [**Blur.frag**](https://github.com/Pecas-Dev/M.I.C.R.O-Graphics-Engine/blob/main/OpenGL2/Assets/Shaders/Blur.frag)
- [**BrightPass.frag**](https://github.com/Pecas-Dev/M.I.C.R.O-Graphics-Engine/blob/main/OpenGL2/Assets/Shaders/BrightPass.frag)
- [**CubeCapture.vert**](https://github.com/Pecas-Dev/M.I.C.R.O-Graphics-Engine/blob/main/OpenGL2/Assets/Shaders/CubeCapture.vert)
- [**Depth.frag**](https://github.com/Pecas-Dev/M.I.C.R.O-Graphics-Engine/blob/main/OpenGL2/Assets/Shaders/Depth.frag)
- [**Depth.vert**](https://github.com/Pecas-Dev/M.I.C.R.O-Graphics-Engine/blob/main/OpenGL2/Assets/Shaders/Depth.vert)
- [**EquirectToCube.frag**](https://github.com/Pecas-Dev/M.I.C.R.O-Graphics-Engine/blob/main/OpenGL2/Assets/Shaders/EquirectToCube.frag)
- [**Irradiance.frag**](https://github.com/Pecas-Dev/M.I.C.R.O-Graphics-Engine/blob/main/OpenGL2/Assets/Shaders/Irradiance.frag)
- [**Post.vert**](https://github.com/Pecas-Dev/M.I.C.R.O-Graphics-Engine/blob/main/OpenGL2/Assets/Shaders/Post.vert)
- [**Prefilter.frag**](https://github.com/Pecas-Dev/M.I.C.R.O-Graphics-Engine/blob/main/OpenGL2/Assets/Shaders/Prefilter.frag)
- [**Skybox.frag**](https://github.com/Pecas-Dev/M.I.C.R.O-Graphics-Engine/blob/main/OpenGL2/Assets/Shaders/Skybox.frag)
- [**Skybox.vert**](https://github.com/Pecas-Dev/M.I.C.R.O-Graphics-Engine/blob/main/OpenGL2/Assets/Shaders/Skybox.vert)
- [**Tonemap.frag**](https://github.com/Pecas-Dev/M.I.C.R.O-Graphics-Engine/blob/main/OpenGL2/Assets/Shaders/Tonemap.frag)

</details>
 <br>

---

## Directory Structure

```
Project Root
├── OpenGL2.sln                      // Visual Studio solution file
├── .gitattributes
├── .gitignore
├── Dependencies                     // External dependencies
│   ├── SDL
│   ├── GLM
│   └── Assimp                       // Open Asset Import Library (multi-format model loading)
│
├── Release                          // Distribution packages
│   ├── MICRO-Setup-x64.exe          // Windows installer (64-bit) - Recommended Download
│   ├── MICRO-Setup-x86.exe          // Windows installer (32-bit)
│   ├── M.I.C.R.O x64.bat            // Run x64 M.I.C.R.O
│   ├── M.I.C.R.O x86.bat            // Run x86 M.I.C.R.O
│   └── Platforms
│       ├── x64                      // Portable 64-bit package (MICRO.exe + DLLs + Assets)
│       │   └── OGL-P2               // Contains the necessary files to run the executable.
│       │       └── Assets
│       │           ├── Models       
│       │           ├── Materials    
│       │           ├── Textures     
│       │           ├── Fonts        
│       │           ├── HDRI         
│       │           ├── Icons        
│       │           └── Shaders
│       │
│       │
│       │
│       └── Win32                    // Portable 32-bit package
│           └── OGL-P2               // Contains the necessary files to run the executable.
│               └── Assets
│                   ├── Models       
│                   ├── Materials    
│                   ├── Textures     
│                   ├── Fonts        
│                   ├── HDRI         
│                   ├── Icons        
│                   └── Shaders      
│
└── OpenGL2
    ├── Assets                       // Stores shaders, models, textures, fonts, and materials.
    │   ├── Models                   // Store model files (`.obj`, `.fbx`, `.gltf`, etc.)             
    │   ├── Materials                // Stores `.mtl` material files 
    │   ├── Textures                 // Stores texture files (`.jpg` or `.png`) 
    │   ├── Fonts                    // Stores the fonts used by the engine
    │   ├── HDRI                     // Stores the HDR files for the skybox
    │   ├── Icons                    // Stores the icons used by the engine
    │   └── Shaders                  // Stores all the shaders that the engine uses
    │
    └── Project                      // Contains the source code for the M.I.C.R.O Graphics Engine.
        ├── include
        └── source

```

---

## Installation & Running

<br>

**Clone the repository:**

```bash
git clone https://github.com/Pecas-Dev/M.I.C.R.O-Graphics-Engine.git
```

### Installer (recommended)

The easiest way to get the engine running is the Windows installer:

1. Grab **`MICRO-Setup-x64.exe`** from the `Release` folder (or `MICRO-Setup-x86.exe` for 32-bit systems).
2. Double-click it. The engine installs to `Program Files\M.I.C.R.O`, adds a **Start Menu** entry and an optional **Desktop shortcut**, and registers a proper uninstaller in **"Add or Remove Programs"**. If you don't have admin rights, the installer offers a per-user install instead (no admin needed).
3. Launch **M.I.C.R.O Graphics Engine** from the Start Menu.

> **SmartScreen note:** Windows may show a *"Windows protected your PC"* screen because the installer isn't code-signed. Click **More info → Run anyway**.

The app is safe to launch from anywhere: at startup `MICRO.exe` calls `SDL_GetBasePath()` and sets its own working directory to the folder the exe lives in, so shaders and assets always resolve no matter how it's launched (Start Menu, Desktop shortcut, or a console with any working directory).

### Portable folder (no installer)

Prefer not to install? Copy the `Release/MICRO-win-x64` (or `MICRO-win-x86`) folder anywhere and run `MICRO.exe` directly, it's fully self-contained (exe + SDL/Assimp DLLs + Assets).

### Rebuilding the installer

The installer is built with [Inno Setup](https://jrsoftware.org/isinfo.php) from `Release/Installer.iss`:

```powershell
cd Release
& "$env:LOCALAPPDATA\Programs\Inno Setup 6\ISCC.exe" /DAppArch=x64 Installer.iss
& "$env:LOCALAPPDATA\Programs\Inno Setup 6\ISCC.exe" /DAppArch=x86 Installer.iss
```

Each command packages the matching `MICRO-win-<arch>` staged folder, so re-copy a freshly built `MICRO.exe` into that folder first if you've rebuilt the engine.

### Building from Source

To build the project from source:

1. Open `OpenGL2.sln` in Visual Studio.
2. Choose your configuration (**Debug/Release**) and platform (**x64/x86**).
3. Build the solution.
4. Run the program.

> The **Release** configuration builds a windowed app named `MICRO.exe` (with the app icon embedded); **Debug** builds `OpenGL2.exe` with a console for log output.

<br>

**Once the program is running:**

- **You can manipulate 3D models and adjust properties like position, scale, rotation, lighting, and textures using the Dear ImGUI UI.**


---

## Controls

- **Right-click drag**: Look around; hold and press **W/A/S/D** to fly (**Q/E** down/up, **Shift** to move faster, scroll wheel tunes fly speed).
- **Left-click drag**: Orbit the camera around the current focus point.
- **Middle-click drag**: Pan the camera.
- **Scroll wheel**: Dolly the camera toward/away from the focus point.
- **Ctrl+K**: Open the command palette. **Ctrl+O**: Load a model. **Ctrl+S**: Save a screenshot. **F**: Frame the model. **T**: Toggle turntable. **?**: Shortcut sheet.
- Use Dear ImGUI to adjust model properties (position, rotation, scale, click a field to type a value directly), toggle lighting and textures, tweak the light's position, color, intensity, ambient strength, and falloff, control the HDR environment/skybox, and adjust exposure/bloom, all in real time.


## Credits

This project was created by _**Pecas Dev**_.
