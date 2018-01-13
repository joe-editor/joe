/*
 *  This file is part of Joe's Own Editor for Windows.
 *  Copyright (c) 2014 John J. Jordan.
 *
 *  Joe's Own Editor for Windows is free software: you can redistribute it 
 *  and/or modify it under the terms of the GNU General Public License as
 *  published by the Free Software Foundation, either version 2 of the 
 *  License, or (at your option) any later version.
 *
 *  Joe's Own Editor for Windows is distributed in the hope that it will
 *  be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
 *  of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Joe's Own Editor for Windows.  If not, see
 *  <http://www.gnu.org/licenses/>.
 */

#include "jwwin.h"
#include <stdio.h>

#include "jwbuiltin.h"
#include "jwres.h"
#include "jwutils.h"
#include "jwglobals.h"

static INT_PTR CALLBACK AboutProc(HWND, UINT, WPARAM, LPARAM);

void jwAboutBox(HWND hwnd)
{
	HMODULE current = NULL;
	if (GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS, (LPCTSTR)jwAboutBox, &current)) {
		DialogBox(current, MAKEINTRESOURCE(IDD_ABOUT), hwnd, AboutProc);
	}
}

static char *getLicenseText()
{
	JFILE *fp;
	char *result = NULL;

	fp = jwfopen(L"*license.txt", L"r");
	if (fp && fp->p) {
		result = (char *)malloc(fp->sz + 1);
		strncpy(result, (char *)fp->p, fp->sz);
		result[fp->sz] = 0;
	}

	if (fp) jwfclose(fp);

	return result;
}

static INT_PTR CALLBACK AboutProc(HWND hwnd, UINT msg,
			      WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
	case WM_INITDIALOG:
		/* Load license */
		{
			char *license = getLicenseText();
			SetDlgItemTextA(hwnd, IDC_LICENSEBOX, license);
			free(license);
		}

		/* Draw icon in picture control */
		{
			HWND pic = GetDlgItem(hwnd, IDC_APPICON);
			HICON ico = LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_MAINICON));

			if (ico) {
				SendMessage(pic, STM_SETICON, (WPARAM)ico, (LPARAM)0);
			}
		}
		return 1;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDWEBSITE) {
			/* Load web browser */
			ShellExecuteA(hwnd, "open",
				"https://joe-editor.sourceforge.io/",
				0, 0, SW_SHOWDEFAULT);
		}
		break;

	case WM_CLOSE:
		EndDialog(hwnd, TRUE);
		break;
	}
	
	return 0;
}

/* Originally from windlg.c */
static INT_PTR CALLBACK NullDlgProc(HWND hwnd, UINT msg,
				WPARAM wParam, LPARAM lParam)
{
    return 0;
}

void defuse_showwindow(void)
{
    /*
     * Work around the fact that the app's first call to ShowWindow
     * will ignore the default in favour of the shell-provided
     * setting.
     */
    {
	HWND hwnd;
	hwnd = CreateDialog(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_ABOUT),
			    NULL, NullDlgProc);
	ShowWindow(hwnd, SW_HIDE);
	SetActiveWindow(hwnd);
	DestroyWindow(hwnd);
    }
}

#define UNI2(x) L ## x
#define UNI(x) UNI2(x)

void jwHelp(HWND hwnd, wchar_t *helpfile)
{
    wchar_t path[MAX_PATH];

    /* Try local file */
    if (!utf8towcs(path, jw_joedata, MAX_PATH)) {
	wcscat(path, L"\\doc\\");
	wcscat(path, helpfile);
	wcscat(path, L".html");
	if (GetFileAttributesW(path) != INVALID_FILE_ATTRIBUTES) {
	    ShellExecuteW(hwnd, L"open", path, 0, 0, SW_SHOWDEFAULT);
	    return;
	}
    }

    /* Try web site */
    wcscpy(path, L"https://joe-editor.sourceforge.io/" UNI(JW_SHORTVERSION) L"/");
    wcscat(path, helpfile);
    wcscat(path, L".html");
    ShellExecuteW(hwnd, L"open", path, 0, 0, SW_SHOWDEFAULT);
}
