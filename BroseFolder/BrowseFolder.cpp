// BrowseFolder.cpp: определяет точку входа для консольного приложения.
//

#include "stdafx.h"
#include "FolderBrowserDialog.h"


int _tmain(int argc, _TCHAR* argv[])
{
	FolderBrowserDialog fbd;

	fbd.Owner = GetConsoleWindow();
	fbd.Flags = BIF_RETURNONLYFSDIRS | BIF_EDITBOX | BIF_NEWDIALOGSTYLE | BIF_NONEWFOLDERBUTTON;
	//fbd.Flags |= BIF_RETURNONLYFSDIRS;
	fbd.Title = _T("Choose a folder please. Or try to create new ;)");

	if (fbd.ShowDialog())
	{
		MessageBox(0, fbd.SelectedPath, _T("Selected path:"),
			MB_OK | MB_ICONINFORMATION);
	}

	return 0;
}

