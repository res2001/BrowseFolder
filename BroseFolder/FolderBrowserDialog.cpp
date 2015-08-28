/*
Copyright Emiliarge
https://msdn.microsoft.com/ru-ru/library/windows/desktop/bb762115%28v=vs.85%29.aspx

Copyright CenterWindow() by Mihai MOGA
http://www.codeproject.com/Tips/250672/CenterWindow-in-WIN
*/

#include "stdafx.h"
#include "FolderBrowserDialog.h"
#include <iostream>

INT CALLBACK BrowseCallbackProc(HWND hwnd, UINT uMsg, LPARAM lParam, LPARAM pData);
BOOL CALLBACK FindTreeViewCallback(HWND hwnd, LPARAM lParam);
BOOL CenterWindow(HWND hwndWindow);

FolderBrowserDialog::FolderBrowserDialog(void)
{
	this->Flags = BIF_USENEWUI;
	this->Owner = 0;
	this->Title = 0;
}

bool FolderBrowserDialog::ShowDialog(void)
{
	BROWSEINFO bi;
	memset(&bi, 0, sizeof(bi));

	bi.hwndOwner = this->Owner;
	bi.lpszTitle = this->Title;
	bi.ulFlags = this->Flags;
	bi.lpfn = BrowseCallbackProc;

	OleInitialize(NULL);

	LPITEMIDLIST pIDL = SHBrowseForFolder(&bi);

	if (pIDL == NULL)
	{
		return false;
	}

	TCHAR *buffer = new TCHAR[MAX_PATH];
	if (!SHGetPathFromIDList(pIDL, buffer) != 0)
	{
		CoTaskMemFree(pIDL);
		return false;
	}
	this->SelectedPath = buffer;

	CoTaskMemFree(pIDL);
	OleUninitialize();
	return true;
}

INT CALLBACK BrowseCallbackProc(HWND hwnd, UINT uMsg, LPARAM lParam, LPARAM pData)
{
	static BOOL bProcessEnsureVisible = FALSE;

	switch (uMsg)
	{
	case BFFM_INITIALIZED:
	{
		bProcessEnsureVisible = FALSE;

		TCHAR buff[MAX_PATH];
		::GetCurrentDirectory(MAX_PATH, buff);
		//TCHAR buff[MAX_PATH - 1] = 0;

		//In case of Windows 7 and later
		//INFO: Correction for the Microsoft bug that doesn't 
		//      scroll the tree-view to the selected item...
		
		OSVERSIONINFOEX osi;
		DWORDLONG dwlConditionMask = 0;
		ZeroMemory(&osi, sizeof(OSVERSIONINFOEX));
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
		
		CenterWindow(hwnd);

		::SendMessage(hwnd, BFFM_SETSELECTION, TRUE, (LPARAM)buff);
	}
	break;

	case BFFM_SELCHANGED:
	{
		if (bProcessEnsureVisible)
		{
			bProcessEnsureVisible = FALSE;

			HWND hWndTree = NULL;
			EnumChildWindows(hwnd, FindTreeViewCallback, (LPARAM)&hWndTree);
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

BOOL CALLBACK FindTreeViewCallback(HWND hwnd, LPARAM lParam)
{
	TCHAR szClassName[MAX_PATH];
	szClassName[0] = 0;
	::GetClassName(hwnd, szClassName, sizeof(szClassName));
	szClassName[MAX_PATH - 1] = 0;

	if (lstrcmpi(szClassName, L"SysTreeView32") == 0)
	{
		HWND* phWnd = (HWND*)lParam;
		if (phWnd)
			*phWnd = hwnd;

		return FALSE;
	}

	return TRUE;
}

BOOL CenterWindow(HWND hwndWindow)
{
	HWND hwndParent;
	RECT rectWindow, rectParent, wa;

	// make the window relative to its parent
	if ((hwndParent = GetParent(hwndWindow)) != NULL)
	{
		GetWindowRect(hwndWindow, &rectWindow);
		GetWindowRect(hwndParent, &rectParent);

		int nWidth = rectWindow.right - rectWindow.left;
		int nHeight = rectWindow.bottom - rectWindow.top;

		int nX = ((rectParent.right - rectParent.left) - nWidth) / 2 + rectParent.left;
		int nY = ((rectParent.bottom - rectParent.top) - nHeight) / 2 + rectParent.top;

		SystemParametersInfo(SPI_GETWORKAREA, 0, &wa, 0);
		//int nScreenWidth = GetSystemMetrics(SM_CXSCREEN);
		//int nScreenHeight = GetSystemMetrics(SM_CYSCREEN);
		int nScreenWidth = wa.right - wa.left;
		int nScreenHeight = wa.bottom - wa.top;
		int nTPWidth = (GetSystemMetrics(SM_CXSCREEN) - nScreenWidth)*2;
		int nTPHeight = (GetSystemMetrics(SM_CYSCREEN) - nScreenHeight)*2;

		std::cout << "nWidth=" << nWidth << "\tnHeight=" << nHeight << "\n";
		std::cout << "rectParent=" << rectParent.left << "\t" << rectParent.right << "\t"
			<< rectParent.top << "\t" << rectParent.bottom << "\n";
		std::cout << "nX=" << nX << "\t" << "nY=" << nY << "\n";
		std::cout << "nScreenWidth=" << nScreenWidth << "\tnScreenHeight=" << nScreenHeight << "\n";
		std::cout << "nTPWidth=" << nTPWidth << "\tnTPHeight=" << nTPHeight << "\n";
		// make sure that the dialog box never moves outside of the screen
		if (nX < 0) nX = 0;
		if (nY < 0) nY = 0;
		if ((nX + nWidth + nTPWidth) > nScreenWidth) nX = nScreenWidth - nWidth - nTPWidth;
		if ((nY + nHeight + nTPHeight) > nScreenHeight) nY = nScreenHeight - nHeight - nTPHeight;

		MoveWindow(hwndWindow, nX, nY, nWidth, nHeight, FALSE);

		return TRUE;
	}

	return FALSE;
}