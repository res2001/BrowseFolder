#include "FolderBrowserDialog.h"
#include "resource.h"

#define READMESIZE	6000

//#pragma comment(linker,"/MERGE:.rdata=.text")
//#pragma comment(linker,"/MERGE:.idata=.text")
//#pragma comment(linker,"/SECTION:.text,EWR")
//#pragma comment(linker,"/ENTRY:main")
//#pragma comment(linker,"/NODEFAULTLIB")

void helpmsg();
UINT my_atoi(wchar_t* str);
int GetCSIDL(const LCID thloc, const DWORD csflags, const wchar_t* str);
HANDLE hStdout = 0;	//STDOUT console hendl
HANDLE hStderr = 0;	//STDERR console hendl
UINT concp = 0;		//Console code page

void main()
{
	//setlocale(LC_ALL, "");
	const wchar_t* errmsg[] = { L"Get command line parameters error./n",	//state==1
		L"Instance FolderBrowseDialog not found.\n",			//state==2
		L"Incorrect set /Description parametr.\n",				//state==3
		L"Incorrect set /Title parameter.\n",					//state==4
		L"Incorrect set /Block parameter.\n",					//state==5
		L"Incorrect set /Center parameter.\n",					//state==6
		L"Incorrect set /CurPath parameter.\n",					//state==7
		L"Incorrect set /RootPath parameter.\n",				//state==8
		L"",
		L"Parameter is incorrect.\n",							//state==10
		L"",
		L"OleInitialize error.\n",								//state==12
		L"",
		L"",													//state==14
		L"Location specified is not part of the file system.\n"	//state==15
	};
	hStdout = ::GetStdHandle(STD_OUTPUT_HANDLE);
	hStderr = ::GetStdHandle(STD_ERROR_HANDLE);
	concp = ::GetConsoleCP();
	int argc = 0;
	wchar_t **argv = 0;
	FolderBrowserDialog* fbd = 0;
	int ret = 1;
	int state = 0;

	while (true)
	{
		state = 1;
		argv = CommandLineToArgvW(GetCommandLineW(), &argc);
		if (!argv)
			break;

		state = 2;
		fbd = FolderBrowserDialog::GetInstance();
		if (!fbd)
			break;

		const LCID thloc = GetThreadLocale();
		const DWORD csflags = NORM_IGNORECASE;
		for (int i = 1; i < argc; ++i)
		{
			if (CompareStringW(thloc, csflags, argv[i], 13, L"/Description:", 13) == CSTR_EQUAL)
			{
				if (!fbd->SetDescription(argv[i] + 13))
				{
					state += 1;
					break;
				}
			}
			else if (CompareStringW(thloc, csflags, argv[i], 7, L"/Title:", 7) == CSTR_EQUAL)
			{
				if (!fbd->SetTitle(argv[i] + 7))
				{
					state += 2;
					break;
				}
			}
			else if (CompareStringW(thloc, csflags, argv[i], 6, L"/Flag:", 6) == CSTR_EQUAL)
			{
				UINT f = my_atoi(argv[i] + 6);
				if ((f & ~0x1f7ff) > 0)
					echo(hStderr, errmsg[3]);
				fbd->SetFlags(f);
			}
			else if (CompareStringW(thloc, csflags, argv[i], 7, L"/Block:", 7) == CSTR_EQUAL)
			{
				wchar_t* str = argv[i] + 7;
				if (CompareStringW(thloc, csflags, str, 6, L"window", 6) == CSTR_EQUAL)
					fbd->SetOwner(GetConsoleWindow());
				else if (CompareStringW(thloc, csflags, str, 4, L"none", 4) == CSTR_EQUAL)
					fbd->SetOwner(0);
				else if (!fbd->SetOwner(FindWindowW(NULL, str)))
				{
					state += 3;
					break;
				}
			}
			else if (CompareStringW(thloc, csflags, argv[i], 8, L"/Center:", 8) == CSTR_EQUAL)
			{
				wchar_t* str = argv[i] + 8;
				if (CompareStringW(thloc, csflags, str, 6, L"window", 6) == CSTR_EQUAL)
					fbd->SetCenterWindow(FolderBrowserDialog::eCenter::CenterWindow);
				else if (CompareStringW(thloc, csflags, str, 6, L"screen", 6) == CSTR_EQUAL)
					fbd->SetCenterWindow(FolderBrowserDialog::eCenter::CenterScreen);
				else if (CompareStringW(thloc, csflags, str, 4, L"none", 4) == CSTR_EQUAL)
					fbd->SetCenterWindow(FolderBrowserDialog::eCenter::NoneCenter);
				else if (!fbd->SetCenterWindow(FolderBrowserDialog::eCenter::CenterHWND, FindWindowW(NULL, str)))
				{
					state += 4;
					break;
				}
			}
			else if (CompareStringW(thloc, csflags, argv[i], 9, L"/CurPath:", 9) == CSTR_EQUAL)
			{
				wchar_t* str = argv[i] + 9;
				bool bret = false;
				int idl = GetCSIDL(thloc, csflags, str);
				if (idl)
					bret = fbd->SetCurdir(idl);
				else
					bret = fbd->SetCurdir(str);
				if (!bret)
				{
					state += 5;
					break;
				}
			}
			else if (CompareStringW(thloc, csflags, argv[i], 10, L"/RootPath:", 10) == CSTR_EQUAL)
			{
				wchar_t* str = argv[i] + 10;
				bool bret = false;
				int idl = GetCSIDL(thloc, csflags, str);
				if (idl)
					bret = fbd->SetRoot(idl);
				else
					bret = fbd->SetRoot(str);
				if(!bret)
				{
					state += 6;
					break;
				}
			}
			else if (CompareStringW(thloc, csflags, argv[i], 5, L"/Help", 5) == CSTR_EQUAL)
			{
				helpmsg();
				state += 7;
				break;
			}
			else
			{
				state += 8;
				break;
			}
		}
		if (state > 2)
			break;

		state = 11 + fbd->ShowDialog();
		if (state == 11)
		{
			echo(hStdout, fbd->SelectedPath);
			echo(hStdout, L"\n");
			ret = 0;
		}
		break;
	}

	if (ret)
	{
		ret = state;
		if(state != 14 && state != 9)
			echo(hStderr, errmsg[state-1]);
	}
	if (argv) 
		LocalFree(argv);
	if (fbd)
		FolderBrowserDialog::DestroyInstance();
	::ExitProcess(ret);
}

UINT my_atoi(wchar_t* str)
{
	if (lstrlenW(str) <= 0)
		return 0;
	UINT ret = 0;
	char* cstr = (char*)str;
	while (*cstr >= '0' && *cstr <= '9' && *(cstr+1) == 0) {
		ret *= 10;
		ret += *cstr - '0';
		cstr += 2;
	}
	return ret;
}

int GetCSIDL(const LCID thloc, const DWORD csflags, const wchar_t* str)
{
	int ret = 0;
	if (CompareStringW(thloc, csflags, str, 7, L"desktop", 7) == CSTR_EQUAL)
		ret = CSIDL_DESKTOPDIRECTORY;
	else if (CompareStringW(thloc, csflags, str, 8, L"computer", 8) == CSTR_EQUAL)
		ret = CSIDL_DRIVES;
	else if (CompareStringW(thloc, csflags, str, 9, L"favorites", 9) == CSTR_EQUAL)
		ret = CSIDL_FAVORITES;
	else if (CompareStringW(thloc, csflags, str, 7, L"appdata", 7) == CSTR_EQUAL)
		ret = CSIDL_APPDATA;
	else if (CompareStringW(thloc, csflags, str, 12, L"localappdata", 12) == CSTR_EQUAL)
		ret = CSIDL_LOCAL_APPDATA;
	else if (CompareStringW(thloc, csflags, str, 9, L"documents", 9) == CSTR_EQUAL)
		ret = CSIDL_MYDOCUMENTS;
	else if (CompareStringW(thloc, csflags, str, 5, L"music", 5) == CSTR_EQUAL)
		ret = CSIDL_MYMUSIC;
	else if (CompareStringW(thloc, csflags, str, 8, L"pictures", 8) == CSTR_EQUAL)
		ret = CSIDL_MYPICTURES;
	else if (CompareStringW(thloc, csflags, str, 5, L"video", 5) == CSTR_EQUAL)
		ret = CSIDL_MYVIDEO;
	else if (CompareStringW(thloc, csflags, str, 7, L"network", 7) == CSTR_EQUAL)
		ret = CSIDL_NETWORK;
	else if (CompareStringW(thloc, csflags, str, 7, L"profile", 7) == CSTR_EQUAL)
		ret = CSIDL_PROFILE;
	else if (CompareStringW(thloc, csflags, str, 12, L"programfiles", 12) == CSTR_EQUAL)
		ret = CSIDL_PROGRAM_FILES;
	else if (CompareStringW(thloc, csflags, str, 7, L"windows", 7) == CSTR_EQUAL)
		ret = CSIDL_WINDOWS;
	else if (CompareStringW(thloc, csflags, str, 6, L"system", 6) == CSTR_EQUAL)
		ret = CSIDL_SYSTEM;
#ifdef _WIN64
	else if (CompareStringW(thloc, csflags, str, 15, L"programfilesx86", 15) == CSTR_EQUAL)
		ret = CSIDL_PROGRAM_FILESX86;
	else if (CompareStringW(thloc, csflags, str, 9, L"systemx86", 9) == CSTR_EQUAL)
		ret = CSIDL_SYSTEMX86;
#endif
	return ret;
}

void helpmsg()
{
	HMODULE hModule = GetModuleHandle(NULL);
	if (!hModule)
	{
		echo(hStderr, L"Unknown module.\n");
		return;
	}
	HRSRC hResInfo = FindResourceA(hModule, MAKEINTRESOURCEA(IDR_README1), MAKEINTRESOURCEA(READMETXT));
	if (hResInfo == NULL)
	{
		echo(hStderr, L"Resource not found.\n");
		return;
	}
	HGLOBAL hResource = LoadResource(hModule, hResInfo);
	if (hResource == NULL)
	{
		echo(hStderr, L"Resource not loaded.\n");
		return;
	}
	DWORD nSize = SizeofResource(hModule, hResInfo);
	LPVOID resource = LockResource(hResource);
	if (resource == NULL)
	{
		echo(hStderr, L"Resource not locked.\n");
		return;
	}
	DWORD chw;
	::WriteFile(hStdout, (char*)resource, nSize, &chw, NULL);
}

void echo(HANDLE h, const wchar_t *str)
{
	const size_t buflength = BUFLEN * sizeof(char);
	char strbuf[BUFLEN];
	DWORD chw = 0;
	size_t strlen = ::lstrlenW(str);
	if (strlen > (buflength - 1)) strlen = buflength - 1;
	int wcl = ::WideCharToMultiByte(concp, 0, str, strlen, strbuf, buflength, NULL, NULL);
	if (wcl <= 0)
	{
		const char *errstr = "Error WideCharToMultibyByte convert string.\n";
		::WriteFile(hStderr, errstr, lstrlenA(errstr), &chw, NULL);
		return;
	}
	::WriteFile (h, strbuf, wcl, &chw, NULL);
	return;
}
