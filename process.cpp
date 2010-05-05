#include "stdafx.h"
#include "process.h"

Process::Process(WTL::CString cmdline) : m_ok(false)
{
    m_cmdline = cmdline;
    m_saAttr.nLength = sizeof(m_saAttr);
    m_saAttr.bInheritHandle = TRUE;
    m_saAttr.lpSecurityDescriptor = NULL;

    /* Create a pipe for the child process's STDOUT */
    if(!CreatePipe(&m_hStdoutR, &m_hStdoutW, &m_saAttr, 0))
        return;

    /* Duplicate stdout sto stderr */
    if (!DuplicateHandle(GetCurrentProcess(), m_hStdoutW, GetCurrentProcess(), &m_hStderrW, 0, TRUE, DUPLICATE_SAME_ACCESS))
        return;

    /* Duplicate the pipe HANDLE */
    if (!DuplicateHandle(GetCurrentProcess(), m_hStdoutR, GetCurrentProcess(), &m_hStdoutRDup, 0, FALSE, DUPLICATE_SAME_ACCESS))
        return;

    ZeroMemory(&m_pi, sizeof(m_pi));
    ZeroMemory(&m_si, sizeof(m_si));
    m_si.cb = sizeof(m_si);
    m_si.hStdOutput = m_hStdoutW;
    m_si.hStdError = m_hStdoutW;
    m_si.wShowWindow = SW_HIDE;
    m_si.dwFlags = STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;

    m_ok = true;
}

DWORD WINAPI Process::OutputThread(LPVOID lpvThreadParam)
{
    Process *pThis = static_cast<Process *>(lpvThreadParam);
    HANDLE Handles[2];
    Handles[0] = pThis->m_pi.hProcess;
    Handles[1] = pThis->m_hEvtStop;

    ResumeThread(pThis->m_pi.hThread);

    while (true)
    {
        DWORD dwRc = WaitForMultipleObjects(2, Handles, FALSE, 100);
        char chBuf[1024];
        do
        {
            DWORD dwRead;
            DWORD dwAvail = 0;
            if (!PeekNamedPipe(pThis->m_hStdoutRDup, NULL, 0, NULL, &dwAvail, NULL) || !dwAvail)
                break;
            if (!ReadFile(pThis->m_hStdoutRDup, chBuf, min(sizeof(chBuf) - 1, dwAvail), &dwRead, NULL) || !dwRead)
                break;
            chBuf[dwRead] = 0;
            pThis->m_buffer += chBuf;
        } while (false);

        if ((dwRc == WAIT_OBJECT_0) || (dwRc == WAIT_OBJECT_0 + 1) || (dwRc == WAIT_FAILED))
            break;
    }
    return 0;
}

Process::~Process()
{
    CloseHandle(m_hStdoutR);
    CloseHandle(m_hStdoutW);
    CloseHandle(m_hStderrW);
    CloseHandle(m_hStdoutRDup);
}

DWORD Process::Exec(WTL::CString &result)
{
    DWORD exitcode;

    if (!m_ok) return -1;

    if (!CreateProcess(NULL, const_cast<LPTSTR>(m_cmdline.GetBuffer(0)),
        NULL,
        NULL,
        TRUE,
        CREATE_NEW_CONSOLE | CREATE_SUSPENDED,
        NULL, NULL,
        &m_si,
        &m_pi))
        return 1;

    m_hEvtStop = CreateEvent(NULL, TRUE, FALSE, NULL);
    DWORD m_dwThreadId;
    HANDLE m_hThread = CreateThread(NULL, 0, OutputThread, (LPVOID) this, 0, &m_dwThreadId);

    WaitForSingleObject(m_pi.hProcess, INFINITE);
    GetExitCodeProcess(m_pi.hProcess, &exitcode);
    CloseHandle(m_pi.hThread);
    CloseHandle(m_pi.hProcess);
    CloseHandle(m_hEvtStop);

    result = m_buffer;
    return exitcode;
}
