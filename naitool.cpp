// naitool.cpp : main source file for naitool.exe
//

#include "stdafx.h"

#include <atlframe.h>
#include <atlctrls.h>
#include <atldlgs.h>

#include "process.h"

#include "resource.h"

#include "MainDlg.h"

CAppModule _Module;

int Run(LPTSTR /*lpstrCmdLine*/ = NULL, int nCmdShow = SW_SHOWDEFAULT)
{
	CMessageLoop theLoop;
	_Module.AddMessageLoop(&theLoop);

	CMainDlg dlgMain;

	if(dlgMain.Create(NULL) == NULL)
	{
		ATLTRACE(_T("Main dialog creation failed!\n"));
		return 0;
	}

	dlgMain.ShowWindow(nCmdShow);

	int nRet = theLoop.Run();

	_Module.RemoveMessageLoop();
	return nRet;
}

int WINAPI _tWinMain(HINSTANCE hInstance, HINSTANCE /*hPrevInstance*/, LPTSTR lpstrCmdLine, int nCmdShow)
{
	HRESULT hRes = ::CoInitialize(NULL);
// If you are running on NT 4.0 or higher you can use the following call instead to 
// make the EXE free threaded. This means that calls come in on a random RPC thread.
//	HRESULT hRes = ::CoInitializeEx(NULL, COINIT_MULTITHREADED);
	ATLASSERT(SUCCEEDED(hRes));

	// this resolves ATL window thunking problem when Microsoft Layer for Unicode (MSLU) is used
	::DefWindowProc(NULL, 0, 0, 0L);

	AtlInitCommonControls(ICC_BAR_CLASSES);	// add flags to support other controls

	hRes = _Module.Init(NULL, hInstance);
	ATLASSERT(SUCCEEDED(hRes));

	int nRet = Run(lpstrCmdLine, nCmdShow);

	_Module.Term();
	::CoUninitialize();

	return nRet;
}

/*
v1: scan /?
McAfee VirusScan for Win32 v5.40.0
Copyright (c) 1992-2008 McAfee, Inc. All rights reserved.
(408) 988-3832  LICENSED COPY - Apr 16 2009

Scan engine v5.4.00 for Win32.
Virus data file v5968 created Apr 30 2010
Scanning for 615622 viruses, trojans and variants.

--

v2: scan /version
McAfee VirusScan Command Line for Win32 Version: 6.0.1.318
Copyright (C) 2009 McAfee, Inc.
(408) 988-3832 EVALUATION COPY - maggio 01 2010

AV Engine version: 5400.1158 for Win32.
Dat set version: 5968 created Apr 30 2010
Scanning for 615622 viruses, trojans and variants.
*/

DWORD WINAPI CMainDlg::DetectVersion(LPVOID lpParameter)
{
    CMainDlg *pThis = static_cast<CMainDlg *> (lpParameter);

    WTL::CString result;
    int pos;
    pThis->m_edit.AppendText(L"Detecting scan executable\r\n");

    Process(L"scan.exe /?").Exec(result);

    pos = result.Find(L"Scan engine v", 0);
    /* v1 TODO */
    if (pos != -1)
    {
        pThis->m_edit.AppendText(L"V1\r\n");
        return 0;
    }

    Process(L"scan.exe /version").Exec(result);

    pos = result.Find(L"McAfee VirusScan Command Line for Win32 Version: ");
    if (pos == -1)
    {
        pThis->m_edit.AppendText(L"Unknown version\r\n");
        return 0;
    }

    pos += sizeof("McAfee VirusScan Command Line for Win32 Version: ") - 1;
    WTL::CString verstr = result.Mid(pos);
    pos = verstr.Find(L"\r");
    pThis->m_version = verstr.Left(pos);

    pos = result.Find(L"Dat set version: ");
    if (pos == -1)
    {
        pThis->m_edit.AppendText(L"Cannot parse dat version\r\n");
        return 0;
    }

    pos += sizeof("Dat set version: ") - 1;
    result = result.Mid(pos);
    pThis->m_datversion = _wtoi(result.GetBuffer(0));

    WTL::CString message;
    message.Format(L"Tool version %s - Dat Version %d\r\n", pThis->m_version,  pThis->m_datversion);
    pThis->m_edit.AppendText(message); 

    return 0;
}
