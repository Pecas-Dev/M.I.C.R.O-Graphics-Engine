#include <Utility/FileDialog.h>

#include <windows.h>
#include <commdlg.h>
#include <shlobj.h>
#include <string>

#pragma comment(lib, "Shell32.lib")
#pragma comment(lib, "Ole32.lib")


namespace
{
	std::string GetDownloadsFolder()
	{
		PWSTR widePath = nullptr;
		std::string result;

		if (SUCCEEDED(SHGetKnownFolderPath(FOLDERID_Downloads, 0, nullptr, &widePath)))
		{
			int length = WideCharToMultiByte(CP_ACP, 0, widePath, -1, nullptr, 0, nullptr, nullptr);

			if (length > 0)
			{
				result.resize(length - 1);
				WideCharToMultiByte(CP_ACP, 0, widePath, -1, &result[0], length, nullptr, nullptr);
			}
		}

		CoTaskMemFree(widePath);

		return result;
	}
}


std::string FileDialog::OpenFile(const char* filter)
{
	char filename[MAX_PATH] = { 0 };

	std::string initialDir = GetDownloadsFolder();

	OPENFILENAMEA ofn;
	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = nullptr;
	ofn.lpstrFilter = filter;
	ofn.lpstrFile = filename;
	ofn.nMaxFile = MAX_PATH;
	ofn.lpstrInitialDir = initialDir.empty() ? nullptr : initialDir.c_str();
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;

	if (GetOpenFileNameA(&ofn))
	{
		return filename;
	}

	return std::string();
}
