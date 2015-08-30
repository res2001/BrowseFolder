#pragma once

#include <Windows.h>
#include <Shlobj.h>

#pragma comment(lib, "Shell32.lib")

extern HANDLE hStderr;
extern const UINT BUFLEN;
extern wchar_t strbufw[];
extern wchar_t curdir[];
void echo(HANDLE h, wchar_t *str);

/*
#ifdef STRICT
	#define MYDLGWNDPROC WNDPROC
#else
	#define MYDLGWNDPROC FARPROC
#endif
*/

class FolderBrowserDialog
{
public:
	static bool ShowDialog();
	static void Init();
	static void Destroy();

	static int Flags;
	static HWND Owner;
	static wchar_t SelectedPath[MAX_PATH];
	static wchar_t *Title;
private:
	FolderBrowserDialog() {};
	~FolderBrowserDialog() {};
	static INT CALLBACK BrowseCallbackProc(HWND hwnd, UINT uMsg, LPARAM lParam, LPARAM pData);
	static BOOL CALLBACK FindTreeViewCallback(HWND hwnd, LPARAM lParam);
	static BOOL CenterWindow(RECT* rectWindow);
	static LRESULT CALLBACK DlgWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	static LRESULT CALLBACK CBTProc(int nCode, WPARAM wParam, LPARAM lParam);

	static WNDPROC OldDlgProc;
	static HWND hFolderDlg;
	static HHOOK hHook;
};
