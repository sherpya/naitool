// MainDlg.h : interface of the CMainDlg class
//
/////////////////////////////////////////////////////////////////////////////

#pragma once

class CMainDlg : public CDialogImpl<CMainDlg>, public CUpdateUI<CMainDlg>,
		public CMessageFilter, public CIdleHandler
{
public:
	enum { IDD = IDD_MAINDLG };

	virtual BOOL PreTranslateMessage(MSG* pMsg)
	{
		return CWindow::IsDialogMessage(pMsg);
	}

	virtual BOOL OnIdle()
	{
		return FALSE;
	}

	BEGIN_UPDATE_UI_MAP(CMainDlg)
	END_UPDATE_UI_MAP()

	BEGIN_MSG_MAP(CMainDlg)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
		COMMAND_ID_HANDLER(IDOK, OnOK)
		COMMAND_ID_HANDLER(IDCANCEL, OnCancel)
        COMMAND_ID_HANDLER(IDC_TEST, OnTest)
	END_MSG_MAP()

// Handler prototypes (uncomment arguments if needed):
//	LRESULT MessageHandler(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
//	LRESULT CommandHandler(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
//	LRESULT NotifyHandler(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/)

	LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		// center the dialog on the screen
		CenterWindow();

		// set icons
		HICON hIcon = AtlLoadIconImage(IDR_MAINFRAME, LR_DEFAULTCOLOR, ::GetSystemMetrics(SM_CXICON), ::GetSystemMetrics(SM_CYICON));
		SetIcon(hIcon, TRUE);
		HICON hIconSmall = AtlLoadIconImage(IDR_MAINFRAME, LR_DEFAULTCOLOR, ::GetSystemMetrics(SM_CXSMICON), ::GetSystemMetrics(SM_CYSMICON));
		SetIcon(hIconSmall, FALSE);

		// register object for message filtering and idle updates
		CMessageLoop* pLoop = _Module.GetMessageLoop();
		ATLASSERT(pLoop != NULL);
		pLoop->AddMessageFilter(this);
		pLoop->AddIdleHandler(this);

		UIAddChildWindowContainer(m_hWnd);

        m_edit = CEdit(GetDlgItem(IDC_EDIT));
        m_progress = CProgressBarCtrl(GetDlgItem(IDC_PROGRESS));

		return TRUE;
	}

	LRESULT OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		// unregister message filtering and idle updates
		CMessageLoop* pLoop = _Module.GetMessageLoop();
		ATLASSERT(pLoop != NULL);
		pLoop->RemoveMessageFilter(this);
		pLoop->RemoveIdleHandler(this);

		return 0;
	}

	LRESULT OnOK(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		// TODO: Add validation code 
		CloseDialog(wID);
		return 0;
	}

	LRESULT OnCancel(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		CloseDialog(wID);
		return 0;
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

    LRESULT OnTest(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
        WTL::CString result;
        int pos;
        m_edit.AppendText(L"Detecting scan executable\r\n");

        Process *p = new Process(L"scan.exe /?");
        p->Prepare();
        p->Exec(result);
        delete p;

        pos = result.Find(L"Scan engine v", 0);
        /* v1 TODO */
        if (pos != -1)
        {
            m_edit.AppendText(L"V1\r\n");
            return 0;
        }

        p = new Process(L"scan.exe /version");
        p->Prepare();
        p->Exec(result);
        delete p;

        pos = result.Find(L"McAfee VirusScan Command Line for Win32 Version: ");
        if (pos == -1)
        {
            m_edit.AppendText(L"Unknown version\r\n");
            return 0;
        }

        pos += sizeof("McAfee VirusScan Command Line for Win32 Version: ") - 1;
        WTL::CString verstr = result.Mid(pos);
        pos = verstr.Find(L"\r");
        verstr = verstr.Left(pos);

        pos = result.Find(L"Dat set version: ");
        if (pos == -1)
        {
            m_edit.AppendText(L"Cannot parse dat version\r\n");
            return 0;
        }

        pos += sizeof("Dat set version: ") - 1;
        result = result.Mid(pos);
        int ver = _wtoi(result.GetBuffer(0));

        WTL::CString message;
        message.Format(L"Tool version %s - Dat Version %d\r\n", verstr, ver);
        m_edit.AppendText(message); 

		return 0;
	}

	void CloseDialog(int nVal)
	{
		DestroyWindow();
		::PostQuitMessage(nVal);
	}

private:
    CProgressBarCtrl m_progress;
    CEdit m_edit;
};
