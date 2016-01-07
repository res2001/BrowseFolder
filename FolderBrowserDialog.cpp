/*
Copyright base FolderBrowserDialog by Emiliarge
https://msdn.microsoft.com/ru-ru/library/windows/desktop/bb762115%28v=vs.85%29.aspx

Copyright fCenterWindow() by Mihai MOGA
http://www.codeproject.com/Tips/250672/CenterWindow-in-WIN

WndProc substitution BrowseForFolder dialog using SetWindowsHookEx for centering relative to the parent window.
Copyright by Remy Lebeau
http://www.delphigroups.info/3/11/54334.html

Using TreeView_Expand for expanded selected directory (standard message BFFM_SETEXPANDED 
do not work when setting flag BIF_NEWDIALOGSTYLE - ibidem)
Copyright by Thomas and Remy Lebeau
http://www.devsuperpage.com/search/Articles.asp?ArtID=1185413
*/

#include "FolderBrowserDialog.h"

FolderBrowserDialog* FolderBrowserDialog::once = 0;

FolderBrowserDialog::FolderBrowserDialog()
	:	
		Flags(BIF_RETURNONLYFSDIRS | BIF_USENEWUI | BIF_NONEWFOLDERBUTTON),
		Owner(GetConsoleWindow()),
		OldDlgProc(0),
		hFolderDlg(0),
		hHook(0),
		hCenter(GetConsoleWindow()),
		root(0),
		isCenter(true)
{
	SelectedPath[0] = 0;
	curdir[0] = 0;
	szClassName[0] = 0;
	Descr[0] = 0;
	Title[0] = 0;
}

FolderBrowserDialog::~FolderBrowserDialog()
{
	if (root)
		::CoTaskMemFree(root);
}

void * FolderBrowserDialog::operator new(size_t size)
{
	return LocalAlloc(LPTR, size);
}

void FolderBrowserDialog::operator delete(void * p)
{
	LocalFree(p);
}

FolderBrowserDialog* FolderBrowserDialog::GetInstance()
{
	if (once) return once;
	once = new FolderBrowserDialog;
	return once;
}

void FolderBrowserDialog::DestroyInstance()
{
	if (once)
	{
		delete once;
		once = 0;
	}
}

bool FolderBrowserDialog::SetFlags(UINT f)
{
	f = f & 0x1f7ff;
	if (f > 0)
		Flags = f;
	return true;
}

bool FolderBrowserDialog::SetDescription(const wchar_t * str)
{
	return SetBuf(str, Descr, BUFLEN);
}

bool FolderBrowserDialog::SetTitle(const wchar_t * str)
{
	return SetBuf(str, Title, BUFLEN);
}

bool FolderBrowserDialog::SetCurdir(const wchar_t * str)
{
	return SetBufFromDir(str, curdir, MAX_WPATH);
}

bool FolderBrowserDialog::SetRoot(const wchar_t * str)
{
	bool ret = false;
	wchar_t* tstr = (wchar_t*)LocalAlloc(LPTR, sizeof(wchar_t)*MAX_WPATH);
	if (tstr)
	{
		if (SetBufFromDir(str, tstr, MAX_WPATH))
			if (SHParseDisplayName(tstr, NULL, &root, NULL, NULL) == S_OK)
				ret = true;
		LocalFree(tstr);
	}
	return ret;
}

bool FolderBrowserDialog::SetRoot(const int r)
{
	if (SHGetSpecialFolderLocation(NULL, r, &root) == S_OK)
		return true;
	return false;
}

bool FolderBrowserDialog::SetBufFromDir(const wchar_t * src, wchar_t* buf, int blen)
{
	DWORD retlen = GetFullPathNameW(src, blen, buf, NULL);
	if (retlen && retlen < MAX_WPATH && PathIsDirectoryW(buf))
		return true;
	return false;
}

bool FolderBrowserDialog::SetBuf(const wchar_t* src, wchar_t* dst, int blen)
{
	int len = lstrlenW(src);
	if (len > 0 && len < blen)
	{
		lstrcpyW(dst, src);
		return true;
	}
	return false;
}

bool FolderBrowserDialog::SetCenterWindow(eCenter en, HWND h)
{
	switch (en)
	{
	case eCenter::CenterWindow:
		isCenter = true;
		hCenter = GetConsoleWindow();
		break;
	case eCenter::CenterScreen:
		isCenter = true;
		hCenter = 0;
		break;
	case eCenter::CenterHWND:
		if (!h || !IsWindow(h))
			return false;
		isCenter = true;
		hCenter = h;
		break;
	case eCenter::NoneCenter:
		isCenter = false;
		break;
	default:
		return false;
	}
	return true;
}

bool FolderBrowserDialog::SetOwner(HWND o)
{
	if (o == 0)
		Owner = 0;
	else if (!IsWindow(o))
		return false;
	else
		Owner = o;
	return true;
}

int FolderBrowserDialog::ShowDialog(void)
{
	int ret = 1;
	int state = 0;
	LPITEMIDLIST pIDL;
	BROWSEINFO bi;
	::SecureZeroMemory(&bi, sizeof(BROWSEINFO));
	bi.pidlRoot = root;
	bi.hwndOwner = Owner;
	bi.lpszTitle = Descr;
	bi.ulFlags = Flags;
	bi.lpfn = FolderBrowserDialog::BrowseCallbackProc;
	while (true)
	{
		state = 1;
		if (::OleInitialize(NULL) != S_OK)
			break;

		if (isCenter)
		{
			state = 2;
			hHook = ::SetWindowsHookEx(WH_CBT, (HOOKPROC)FolderBrowserDialog::CBTProc, NULL, ::GetCurrentThreadId());
			if (!hHook)
				echo(hStderr, L"Set hook error! It is impossible to center FolderBrowseDialog.\n");
		}

		state = 3;
		pIDL = ::SHBrowseForFolder(&bi);
		if (pIDL == NULL)
			break;

		state = 4;
		if (::SHGetPathFromIDList(pIDL, SelectedPath))
			ret = 0;
		break;
	}
	if (hHook)
	{
		::UnhookWindowsHookEx(hHook);
		hHook = 0;
	}
	if (ret)
		ret = state;
	if (state >= 3)
		::CoTaskMemFree(pIDL);
	if (state >= 1)
		::OleUninitialize();
	return ret;
}

INT CALLBACK FolderBrowserDialog::BrowseCallbackProc(HWND hwnd, UINT uMsg, LPARAM lParam, LPARAM pData)
{
	static BOOL bProcessEnsureVisible = FALSE;

	switch(uMsg)
	{
		case BFFM_INITIALIZED:
		{
			//In case of Windows 7 and later
			//INFO: Correction for the Microsoft bug that doesn't 
			//      scroll the tree-view to the selected item...
			OSVERSIONINFOEX osi;
			DWORDLONG dwlConditionMask = 0;
			::SecureZeroMemory(&osi, sizeof(OSVERSIONINFOEX));
			osi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
			osi.dwMajorVersion = 6;
			osi.dwMinorVersion = 1;
			VER_SET_CONDITION(dwlConditionMask, VER_MAJORVERSION, VER_GREATER_EQUAL);
			VER_SET_CONDITION(dwlConditionMask, VER_MINORVERSION, VER_GREATER_EQUAL);
			//if (GetVersionEx(&osi) && osi.dwMajorVersion >= 6 && osi.dwMinorVersion >= 1)
			if (VerifyVersionInfo(&osi, VER_MAJORVERSION | VER_MINORVERSION, dwlConditionMask))
				bProcessEnsureVisible = TRUE;

			FolderBrowserDialog* bfd = FolderBrowserDialog::GetInstance();
			if (!bfd)
				break;
			if (bfd->Title[0])
				SetWindowTextW(hwnd,bfd->Title);

			if(!bfd->curdir[0])
				::SendMessageW(hwnd, BFFM_SETSELECTIONW, TRUE, (LPARAM)bfd->curdir);
			else if (bfd->root)
				::SendMessageW(hwnd, BFFM_SETSELECTIONW, FALSE, (LPARAM)bfd->root);
			return 1;
		}
		case BFFM_SELCHANGED:
			if (bProcessEnsureVisible)
			{
				bProcessEnsureVisible = FALSE;
				HWND hTreeView = 0;
				::EnumChildWindows(hwnd, FolderBrowserDialog::FindTreeViewCallback, (LPARAM)&hTreeView);
				if (hTreeView)
				{
					HTREEITEM hItm = TreeView_GetSelection(hTreeView);
					if (hItm)
					{
						TreeView_EnsureVisible(hTreeView, hItm);
						TreeView_Expand(hTreeView, hItm, TVE_EXPAND);
					}
				}
			}
			break;
	}
	return 0;
}

BOOL CALLBACK FolderBrowserDialog::FindTreeViewCallback(HWND hwnd, LPARAM lParam)
{
	FolderBrowserDialog* bfd = FolderBrowserDialog::GetInstance();
	if (!bfd)
		return TRUE;
	int clen = ::GetClassNameW(hwnd, bfd->szClassName, MAX_PATH - 1);
	if (!clen)
		return TRUE;

	if (::lstrcmpiW(bfd->szClassName, L"SysTreeView32") == 0)
	{
		HWND* phWnd = (HWND*)lParam;
		if (phWnd)
			*phWnd = hwnd;
		return FALSE;
	}

	return TRUE;
}

BOOL FolderBrowserDialog::fCenterWindow(RECT* rectWindow)
{
	RECT rectParent, wa;
	int nWidth, nHeight, nX, nY, nScreenWidth, nScreenHeight;
	nWidth = rectWindow->right - rectWindow->left;
	nHeight = rectWindow->bottom - rectWindow->top;
	::SystemParametersInfo(SPI_GETWORKAREA, 0, &wa, 0);
	nScreenWidth = wa.right - wa.left;
	nScreenHeight = wa.bottom - wa.top;
	if (hCenter)
	{
		::GetWindowRect(hCenter, &rectParent);
		nX = ((rectParent.right - rectParent.left) - nWidth) / 2 + rectParent.left;
		nY = ((rectParent.bottom - rectParent.top) - nHeight) / 2 + rectParent.top;
	}
	else
	{
		nX = (nScreenWidth - nWidth) / 2 + wa.left;
		nY = (nScreenHeight - nHeight) / 2 + wa.top;
	}
	// make sure that the dialog box never moves outside of the screen
	if (nX < 0) nX = 0;
	if (nY < 0) nY = 0;
	if ((nX + nWidth) > nScreenWidth) nX = nScreenWidth - nWidth;
	if ((nY + nHeight) > nScreenHeight) nY = nScreenHeight - nHeight;

	rectWindow->top = nY;
	rectWindow->bottom = nY + nHeight;
	rectWindow->left = nX;
	rectWindow->right = nX + nWidth;

	return TRUE;
}

LRESULT CALLBACK FolderBrowserDialog::DlgWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	static bool isCenterOnce = true;
	FolderBrowserDialog* bfd = FolderBrowserDialog::GetInstance();
	if (!bfd) 
		return 0;
	if (hwnd==bfd->hFolderDlg && uMsg == WM_SHOWWINDOW && wParam && !lParam && isCenterOnce)
	{
		RECT dr;
		isCenterOnce = false;
		::GetWindowRect(hwnd, &dr);
		if(bfd->fCenterWindow(&dr))
			::SetWindowPos(bfd->hFolderDlg, HWND_TOP, dr.left, dr.top, dr.right - dr.left, dr.bottom - dr.top, 0);
	}
	return ::CallWindowProc(bfd->OldDlgProc, hwnd, uMsg, wParam, lParam);

}

LRESULT CALLBACK FolderBrowserDialog::CBTProc(int nCode, WPARAM wParam, LPARAM lParam)
{
	FolderBrowserDialog* bfd = FolderBrowserDialog::GetInstance();
	if (!bfd)
		return 0;
	switch (nCode)
	{
		case HCBT_CREATEWND:
		{
			HWND hDlg = (HWND)wParam;
			CBT_CREATEWND *lpcw = (CBT_CREATEWND*)lParam;
			if(((lpcw->lpcs->style & 0x80c00080L) == 0x80c00080L) && 
				((lpcw->lpcs->style & 0x8000000L) == 0) &&
				((lpcw->lpcs->style & 0x2000000L) == 0))
			{
				bfd->hFolderDlg = hDlg;
				bfd->OldDlgProc = (WNDPROC) ::SetWindowLong(hDlg, GWL_WNDPROC, (LONG)FolderBrowserDialog::DlgWndProc);
			} 
			break;
		}
		case HCBT_DESTROYWND:
		{
			if (bfd->hFolderDlg && (bfd->hFolderDlg == (HWND)wParam))
			{
				::SetWindowLong(bfd->hFolderDlg, GWL_WNDPROC, (LONG)bfd->OldDlgProc);
				bfd->hFolderDlg = NULL;
			}
			break;
		}
	}
	return ::CallNextHookEx(bfd->hHook, nCode, wParam, lParam);
}
