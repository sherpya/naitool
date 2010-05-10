#include "stdafx.h"
#include "process.h"

Process::Process(WTL::CString cmdline) : m_ok(false), m_buffer(L""), m_cmdline(cmdline)
{
    m_saAttr.nLength = sizeof(m_saAttr);
    m_saAttr.bInheritHandle = TRUE;
    m_saAttr.lpSecurityDescriptor = NULL;

    if(!CreatePipe(&m_pipe, &m_child, &m_saAttr, 0))
        return;

    ZeroMemory(&m_pi, sizeof(m_pi));
    ZeroMemory(&m_si, sizeof(m_si));
    m_si.cb = sizeof(m_si);
    m_si.hStdInput = INVALID_HANDLE_VALUE;
    m_si.hStdOutput = m_child;
    m_si.hStdError = m_child;
    m_si.dwFlags = STARTF_USESTDHANDLES;

    m_ok = true;
}

DWORD WINAPI Process::OutputThread(LPVOID lpvThreadParam)
{
    Process *pThis = static_cast<Process *>(lpvThreadParam);
    DWORD res, dwRead, dwAvail;

    ResumeThread(pThis->m_pi.hThread);

    while (true)
    {
        char chBuf[1024];
        switch ((res = WaitForSingleObject(pThis->m_pi.hProcess, 100)))
        {
            case WAIT_OBJECT_0:
            case STATUS_TIMEOUT:
                    while (PeekNamedPipe(pThis->m_pipe, NULL, 0, NULL, &dwAvail, NULL) && (dwAvail > 0))
                    {
                        if (ReadFile(pThis->m_pipe, chBuf, min(sizeof(chBuf) - 1, dwAvail), &dwRead, NULL) && (dwRead > 0))
                        {
                            chBuf[dwRead] = 0;
                            pThis->m_buffer += chBuf;
                        }
                    }
                if (res == WAIT_OBJECT_0) return 0;
                break;
            default:
                return 1;
        }
    }
    return 0;
}

DWORD Process::Exec(WTL::CString &result)
{
    DWORD exitcode;

    if (!m_ok) return -1;

    if (!CreateProcess(NULL, const_cast<LPTSTR>(m_cmdline.GetBuffer(0)),
        NULL,
        NULL,
        TRUE,
        CREATE_NO_WINDOW | CREATE_SUSPENDED,
        NULL, NULL,
        &m_si,
        &m_pi))
        return 1;

    DWORD m_dwThreadId;
    HANDLE m_hThread = CreateThread(NULL, 0, OutputThread, (LPVOID) this, 0, &m_dwThreadId);

    WaitForSingleObject(m_hThread, INFINITE);
    GetExitCodeProcess(m_pi.hProcess, &exitcode);
    CloseHandle(m_child);
    CloseHandle(m_pipe);
    CloseHandle(m_pi.hThread);
    CloseHandle(m_pi.hProcess);

    result = m_buffer;
    return exitcode;
}
