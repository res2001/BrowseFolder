#pragma once
#pragma warning( disable : 4091 )

#include <Windows.h>
#include <Shlobj.h>
#include <Shlwapi.h>

#pragma comment(lib, "Shell32.lib")

const UINT BUFLEN = MAX_PATH;
const UINT MAX_WPATH = 32767;
extern HANDLE hStderr;
void echo(HANDLE h, const wchar_t *str);

class FolderBrowserDialog
{
public:
	enum eCenter {CenterWindow, CenterScreen, CenterHWND, NoneCenter};
	static FolderBrowserDialog* GetInstance();
	static void DestroyInstance();
	void *operator new(size_t size);
	void operator delete(void *p);

	int ShowDialog();
	bool SetFlags(UINT f);
	bool SetDescription(const wchar_t* str);
	bool SetTitle(const wchar_t* str);
	bool SetCurdir(const wchar_t* str);
	bool SetCurdir(const int idl);
	bool SetRoot(const wchar_t* str);
	bool SetRoot(const int r);
	bool SetBufFromDir(const wchar_t * src, wchar_t * buf, int blen);
	bool SetCenterWindow(eCenter en, HWND h=0);
	bool SetOwner(HWND o);
	wchar_t SelectedPath[MAX_PATH];
private:
	FolderBrowserDialog();
	~FolderBrowserDialog();

	static INT CALLBACK BrowseCallbackProc(HWND hwnd, UINT uMsg, LPARAM lParam, LPARAM pData);
	static BOOL CALLBACK FindTreeViewCallback(HWND hwnd, LPARAM lParam);
	static LRESULT CALLBACK DlgWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	static LRESULT CALLBACK CBTProc(int nCode, WPARAM wParam, LPARAM lParam);
	static FolderBrowserDialog* once;

	BOOL fCenterWindow(RECT* rectWindow);
	bool SetBuf(const wchar_t* src, wchar_t* dst, int blen);

	WNDPROC OldDlgProc;
	HWND hFolderDlg;
	HHOOK hHook;
	UINT Flags;
	HWND Owner;
	HWND hCenter;
	bool isCenter;
	PIDLIST_ABSOLUTE root;
	wchar_t Descr[BUFLEN];
	wchar_t Title[BUFLEN];
	wchar_t szClassName[MAX_PATH];
	wchar_t curdir[MAX_WPATH];
};
