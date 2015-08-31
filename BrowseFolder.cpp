// BrowseFolder.cpp: определяет точку входа для консольного приложения.
//

#include "stdafx.h"
#include "FolderBrowserDialog.h"

const UINT BUFLEN = MAX_PATH;
HANDLE hStdout = 0;	//STDOUT console hendl
HANDLE hStderr = 0;	//STDERR console hendl
UINT concp = 0;		//Console code page
wchar_t curdir[MAX_PATH];
wchar_t strbufw[BUFLEN];
char strbuf[BUFLEN];
size_t bufwlength = 0;
size_t buflength = 0;

int main(int argc, _TCHAR* argv[])
{
	hStdout = ::GetStdHandle(STD_OUTPUT_HANDLE);
	hStderr = ::GetStdHandle(STD_ERROR_HANDLE);
	concp = ::GetConsoleCP();
	bufwlength = BUFLEN * sizeof(wchar_t);
	buflength = BUFLEN * sizeof(char);
	::GetCurrentDirectory(MAX_PATH, curdir);

	//setlocale(LC_ALL, "");
        FolderBrowserDialog::Init();
	FolderBrowserDialog::Owner = ::GetConsoleWindow();
	FolderBrowserDialog::Flags = BIF_RETURNONLYFSDIRS | BIF_EDITBOX | BIF_NEWDIALOGSTYLE | BIF_NONEWFOLDERBUTTON;
	FolderBrowserDialog::Title = L"Choose a folder please. Or try to create new ;)";

	if (FolderBrowserDialog::ShowDialog())
	{
		echo(hStdout, FolderBrowserDialog::SelectedPath);
	        FolderBrowserDialog::Destroy();
		::ExitProcess(0);
	}
        FolderBrowserDialog::Destroy();
	::ExitProcess(1);
}

void echo(HANDLE h, wchar_t *str)
{
	DWORD chw = 0;
	size_t strlen = lstrlenW(str);
	if (strlen > (buflength - 1)) strlen = buflength - 1;
	int wcl = ::WideCharToMultiByte(concp, 0, str, strlen, strbuf, buflength, NULL, NULL);
	if (wcl <= 0)
	{
		static const char *errstr = "Error WideCharToMultibyByte convert string.\n";
		::WriteFile(hStderr, errstr, lstrlenA(errstr), &chw, NULL);
		return;
	}
	::WriteFile (h, strbuf, wcl, &chw, NULL);
	return;
}
