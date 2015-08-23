#pragma once

#include <Windows.h>
#include <Shlobj.h>

#pragma comment(lib, "Shell32.lib")

class FolderBrowserDialog
{
public:
	FolderBrowserDialog(void);

	int Flags;
	HWND Owner;
	TCHAR *SelectedPath;
	TCHAR *Title;

	bool ShowDialog();
};
