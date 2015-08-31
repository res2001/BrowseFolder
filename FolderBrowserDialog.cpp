/*
Copyright Emiliarge
https://msdn.microsoft.com/ru-ru/library/windows/desktop/bb762115%28v=vs.85%29.aspx

Copyright CenterWindow() by Mihai MOGA
http://www.codeproject.com/Tips/250672/CenterWindow-in-WIN

Using SetWindowsHookEx for replace WndProc for BrowseForFolder dialog box
Copyright Remy Lebeau
http://www.delphigroups.info/3/11/54334.html
*/

#include "stdafx.h"
#include "FolderBrowserDialog.h"
#ifdef DEBUG
	#include <iostream>
	#include <iomanip>
#endif

int FolderBrowserDialog::Flags = BIF_USENEWUI;
HWND FolderBrowserDialog::Owner = 0;
wchar_t FolderBrowserDialog::SelectedPath[MAX_PATH];
wchar_t *FolderBrowserDialog::Title = 0;
wchar_t *FolderBrowserDialog::szClassName = 0;
WNDPROC FolderBrowserDialog::OldDlgProc = 0;
HWND FolderBrowserDialog::hFolderDlg = 0;
HHOOK FolderBrowserDialog::hHook = 0;

void FolderBrowserDialog::Init()
{
	Flags = BIF_USENEWUI;
	Owner = 0;
	SelectedPath[0] = 0;
	Title = 0;
	OldDlgProc = 0;
	hFolderDlg = 0;
	hHook = 0;
	if (szClassName)
	{
		LocalFree(szClassName);
		szClassName = 0;
	}
}

void FolderBrowserDialog::Destroy()
{
	Init();
}

bool FolderBrowserDialog::ShowDialog(void)
{
	BROWSEINFO bi;
	::SecureZeroMemory(&bi, sizeof(BROWSEINFO));

	bi.hwndOwner = Owner;
	bi.lpszTitle = Title;
	bi.ulFlags = Flags;
	bi.lpfn = BrowseCallbackProc;

	::OleInitialize(NULL);
	
	if (Owner)
	{
		hHook = ::SetWindowsHookEx(WH_CBT, (HOOKPROC)FolderBrowserDialog::CBTProc, NULL, ::GetCurrentThreadId());
		if (!hHook)
		{
			echo(hStderr, L"Error set hook!\n");
			return false;
		}
	}
	
	LPITEMIDLIST pIDL = ::SHBrowseForFolder(&bi);

	if (pIDL == NULL)
	{
		echo(hStderr, L"Error IDL!\n");
		return false;
	}

	if (!::SHGetPathFromIDList(pIDL, FolderBrowserDialog::SelectedPath) != 0)
	{
		echo(hStderr, L"Location specified is not part of the file system!\n");
		::CoTaskMemFree(pIDL);
		return false;
	}
	
	if (Owner)
	{
		::UnhookWindowsHookEx(hHook);
		hHook = 0;
	}
	
	::CoTaskMemFree(pIDL);
	::OleUninitialize();
	return true;
}

INT CALLBACK FolderBrowserDialog::BrowseCallbackProc(HWND hwnd, UINT uMsg, LPARAM lParam, LPARAM pData)
{
	static BOOL bProcessEnsureVisible = FALSE;

	switch(uMsg)
	{
		case BFFM_INITIALIZED:
		{
			bProcessEnsureVisible = FALSE;

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
			{
				bProcessEnsureVisible = TRUE;
			}
			::SendMessage(hwnd, BFFM_SETSELECTION, TRUE, (LPARAM)curdir);
			break;
		}
		case BFFM_SELCHANGED:
		{
			if (bProcessEnsureVisible)
			{
				bProcessEnsureVisible = FALSE;

				HWND hWndTree = NULL;
				::EnumChildWindows(hwnd, FindTreeViewCallback, (LPARAM)&hWndTree);
				if (hWndTree)
				{
					HTREEITEM hItm = TreeView_GetSelection(hWndTree);
					if (hItm)
					{
						TreeView_EnsureVisible(hWndTree, hItm);
					}
				}
			}
		}
		break;
	}
	return 0;
}

BOOL CALLBACK FolderBrowserDialog::FindTreeViewCallback(HWND hwnd, LPARAM lParam)
{
	if (!szClassName)
	{
		szClassName = (wchar_t*)LocalAlloc(LPTR, MAX_PATH*sizeof(wchar_t));
		if (!szClassName) return FALSE;
	}
	//wchar_t szClassName[MAX_PATH];
	::GetClassName(hwnd, szClassName, MAX_PATH*sizeof(wchar_t) - 1);
	szClassName[MAX_PATH - 1] = 0;

	if (lstrcmpi(szClassName, L"SysTreeView32") == 0)
	{
		HWND* phWnd = (HWND*)lParam;
		if (phWnd)
			*phWnd = hwnd;
		if (szClassName)
		{
			LocalFree(szClassName);
			szClassName = 0;
		}
		return FALSE;
	}

	return TRUE;
}

BOOL FolderBrowserDialog::CenterWindow(RECT* rectWindow)
{
	if (Owner != NULL)
	{
		RECT rectParent, wa;
		::GetWindowRect(Owner, &rectParent);
		
		int nWidth = rectWindow->right - rectWindow->left;
		int nHeight = rectWindow->bottom - rectWindow->top;

		int nX = ((rectParent.right - rectParent.left) - nWidth) / 2 + rectParent.left;
		int nY = ((rectParent.bottom - rectParent.top) - nHeight) / 2 + rectParent.top;

		::SystemParametersInfo(SPI_GETWORKAREA, 0, &wa, 0);
		int nScreenWidth = wa.right - wa.left;
		int nScreenHeight = wa.bottom - wa.top;

		// make sure that the dialog box never moves outside of the screen
		if (nX < 0) nX = 0;
		if (nY < 0) nY = 0;
		if ((nX + nWidth) > nScreenWidth) nX = nScreenWidth - nWidth;
		if ((nY + nHeight) > nScreenHeight) nY = nScreenHeight - nHeight;

		//::MoveWindow(hFolderDlg, nX, nY, nWidth, nHeight, FALSE);
		rectWindow->top = nY;
		rectWindow->bottom = nY + nHeight;
		rectWindow->left = nX;
		rectWindow->right = nX + nWidth;

		//std::cout << std::dec << "nX=" << nX << "\t" << "nY=" << nY << "\n";
		//std::cout << "nWidth=" << nWidth << "\tnHeight=" << nHeight << "\n";
		//std::cout << "rectParent Left=" << rectParent.left << "\tRight=" << rectParent.right << "\ttop="
		//	<< rectParent.top << "\tbottom" << rectParent.bottom << "\n";
		//std::cout << "nScreenWidth=" << nScreenWidth << "\tnScreenHeight=" << nScreenHeight << "\n";
		//std::cout << "HWNDDLG=0x" << std::hex << hFolderDlg << "\n";
		//std::cout << "HWNDOWNER=0x" << std::hex << Owner << "\n";

		return TRUE;
	}

	return FALSE;
}

LRESULT CALLBACK FolderBrowserDialog::DlgWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (uMsg == WM_SHOWWINDOW && wParam)
	{
		RECT dr;
		::GetWindowRect(hwnd, &dr);
		if(CenterWindow(&dr))
			::SetWindowPos(hFolderDlg, HWND_TOP, dr.left, dr.top, dr.right - dr.left, dr.bottom - dr.top, 0);
	}
	return ::CallWindowProc(OldDlgProc, hwnd, uMsg, wParam, lParam);

}

LRESULT CALLBACK FolderBrowserDialog::CBTProc(int nCode, WPARAM wParam, LPARAM lParam)
{
	switch (nCode)
	{
		case HCBT_CREATEWND:
		{
			HWND hDlg = (HWND)wParam;
			CBT_CREATEWND *lpcw = (CBT_CREATEWND*)lParam;
			if (lpcw->lpcs->hwndParent == Owner)
			{
				hFolderDlg = hDlg;
				OldDlgProc = (WNDPROC) ::SetWindowLong(hDlg, GWL_WNDPROC, (LONG)FolderBrowserDialog::DlgWndProc);
			}
			break;
		}
		case HCBT_DESTROYWND:
		{
			if (hFolderDlg && (hFolderDlg == (HWND)wParam))
			{
				::SetWindowLong(hFolderDlg, GWL_WNDPROC, (LONG)OldDlgProc);
				hFolderDlg = NULL;
			}
			break;
		}
	}
	return ::CallNextHookEx(hHook, nCode, wParam, lParam);
}
