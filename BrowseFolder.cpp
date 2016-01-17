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
			int arglen = lstrlenW(argv[i]);
			if (arglen > 13 && CompareStringW(thloc, csflags, argv[i], 13, L"/Description:", 13) == CSTR_EQUAL)
			{
				if (!fbd->SetDescription(argv[i] + 13))
				{
					state += 1;
					break;
				}
			}
			else if (arglen > 7 && CompareStringW(thloc, csflags, argv[i], 7, L"/Title:", 7) == CSTR_EQUAL)
			{
				if (!fbd->SetTitle(argv[i] + 7))
				{
					state += 2;
					break;
				}
			}
			else if (arglen > 6 && CompareStringW(thloc, csflags, argv[i], 6, L"/Flag:", 6) == CSTR_EQUAL)
			{
				UINT f = my_atoi(argv[i] + 6);
				if ((f & ~0x1f7ff) > 0)
					echo(hStderr, errmsg[3]);
				fbd->SetFlags(f);
			}
			else if (arglen > 7 && CompareStringW(thloc, csflags, argv[i], 7, L"/Block:", 7) == CSTR_EQUAL)
			{
				wchar_t* str = argv[i] + 7;
				if (arglen == 13 && CompareStringW(thloc, csflags, str, 6, L"window", 6) == CSTR_EQUAL)
					fbd->SetOwner(GetConsoleWindow());
				else if (arglen == 11 && CompareStringW(thloc, csflags, str, 4, L"none", 4) == CSTR_EQUAL)
					fbd->SetOwner(0);
				else if (!fbd->SetOwner(FindWindowW(NULL, str)))
				{
					state += 3;
					break;
				}
			}
			else if (arglen > 8 && CompareStringW(thloc, csflags, argv[i], 8, L"/Center:", 8) == CSTR_EQUAL)
			{
				wchar_t* str = argv[i] + 8;
				if (arglen == 14 && CompareStringW(thloc, csflags, str, 6, L"window", 6) == CSTR_EQUAL)
					fbd->SetCenterWindow(FolderBrowserDialog::eCenter::CenterWindow);
				else if (arglen == 14 && CompareStringW(thloc, csflags, str, 6, L"screen", 6) == CSTR_EQUAL)
					fbd->SetCenterWindow(FolderBrowserDialog::eCenter::CenterScreen);
				else if (arglen == 12 && CompareStringW(thloc, csflags, str, 4, L"none", 4) == CSTR_EQUAL)
					fbd->SetCenterWindow(FolderBrowserDialog::eCenter::NoneCenter);
				else if (!fbd->SetCenterWindow(FolderBrowserDialog::eCenter::CenterHWND, FindWindowW(NULL, str)))
				{
					state += 4;
					break;
				}
			}
			else if (arglen > 9 && CompareStringW(thloc, csflags, argv[i], 9, L"/CurPath:", 9) == CSTR_EQUAL)
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
			else if (arglen > 10 && CompareStringW(thloc, csflags, argv[i], 10, L"/RootPath:", 10) == CSTR_EQUAL)
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
			else if (arglen == 5 && CompareStringW(thloc, csflags, argv[i], 5, L"/Help", 5) == CSTR_EQUAL)
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
			::WriteFile(hStdout, "\n", 2, NULL, NULL);
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
	const wchar_t *astr[] = {L"desktop", L"computer", L"favorites", L"appdata", L"localappdata", 
						L"documents", L"music", L"pictures", L"video", L"network", L"profile", 
						L"programfiles", L"windows", L"system"
#ifdef _WIN64
						,L"programfilesx86", L"systemx86"
#endif
	};
	const int alen[] = {	7, 8, 9, 7, 12, 9, 5, 8, 5, 7, 7, 12, 7, 6
#ifdef _WIN64
					,15, 9
#endif
	};
	const int aret[] = { CSIDL_DESKTOPDIRECTORY, CSIDL_DRIVES, CSIDL_FAVORITES, CSIDL_APPDATA, CSIDL_LOCAL_APPDATA,
					CSIDL_MYDOCUMENTS, CSIDL_MYMUSIC, CSIDL_MYPICTURES, CSIDL_MYVIDEO, CSIDL_NETWORK, CSIDL_PROFILE,
					CSIDL_PROGRAM_FILES, CSIDL_WINDOWS, CSIDL_SYSTEM
#ifdef _WIN64
					,CSIDL_PROGRAM_FILESX86, CSIDL_SYSTEMX86
#endif
	};
#ifdef _WIN64
	const int count = 16;
#else
	const int count = 14;
#endif
	int l = lstrlenW(str);
	for (int i = 0; i < count; ++i)
		if (l == alen[i] && CompareStringW(thloc, csflags, str, alen[i], astr[i], alen[i]) == CSTR_EQUAL)
			return aret[i];
	return 0;
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
	::WriteFile(hStdout, (char*)resource, nSize, NULL, NULL);
}

void echo(HANDLE h, const wchar_t *str)
{
	const size_t buflength = BUFLEN * sizeof(char);
	char strbuf[BUFLEN];
	size_t strlen = ::lstrlenW(str);
	if (strlen > (buflength - 1)) strlen = buflength - 1;
	int wcl = ::WideCharToMultiByte(concp, 0, str, strlen, strbuf, buflength, NULL, NULL);
	if (wcl <= 0)
	{
		const char *errstr = "Error WideCharToMultibyByte convert string.\n";
		::WriteFile(hStderr, errstr, lstrlenA(errstr), NULL, NULL);
		return;
	}
	::WriteFile (h, strbuf, wcl, NULL, NULL);
	return;
}
