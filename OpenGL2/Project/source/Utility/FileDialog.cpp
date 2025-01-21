#include <Utility/FileDialog.h>

#include <windows.h>
#include <commdlg.h>
#include <string>


std::string FileDialog::OpenFile(const char* filter)
{
	char filename[MAX_PATH] = { 0 };

	OPENFILENAMEA ofn;
	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = nullptr;
	ofn.lpstrFilter = filter;
	ofn.lpstrFile = filename;
	ofn.nMaxFile = MAX_PATH;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;

	if (GetOpenFileNameA(&ofn))
	{
		return filename;
	}

	return std::string();
}