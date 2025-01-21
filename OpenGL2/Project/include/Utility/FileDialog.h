#pragma once

#include <string>


class FileDialog
{
public:
    static std::string OpenFile(const char* filter = "OBJ Files\0*.obj\0All Files\0*.*\0");
};