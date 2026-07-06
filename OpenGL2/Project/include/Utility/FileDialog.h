#pragma once

#include <string>


class FileDialog
{
public:
    static std::string OpenFile(const char* filter = "3D Models\0*.obj;*.fbx;*.gltf;*.glb;*.dae;*.stl;*.ply;*.3ds\0OBJ Files\0*.obj\0All Files\0*.*\0");
};