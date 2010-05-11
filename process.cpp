//
// McAfee Updater Tool
//
// Copyright (c) 2010 Gianluigi Tiesi <sherpya@netfarm.it>
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Library General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
// Library General Public License for more details.
//
// You should have received a copy of the GNU Library General Public
// License along with this software; if not, write to the
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
//

#include "stdafx.h"
#include "process.h"

const PROCESS_INFORMATION pi_init = { INVALID_HANDLE_VALUE, INVALID_HANDLE_VALUE, 0, 0 };

Process::Process(WTL::CString cmdline) : m_ok(false), m_buffer(L""), m_cmdline(cmdline), m_pi(pi_init)
{
    m_saAttr.nLength = sizeof(m_saAttr);
    m_saAttr.bInheritHandle = TRUE;
    m_saAttr.lpSecurityDescriptor = NULL;

    if(!CreatePipe(&m_pipe, &m_child, &m_saAttr, 0))
        return;

    ZeroMemory(&m_si, sizeof(m_si));
    m_si.cb = sizeof(m_si);
    m_si.hStdInput = INVALID_HANDLE_VALUE;
    m_si.hStdOutput = m_child;
    m_si.hStdError = m_child;
    m_si.dwFlags = STARTF_USESTDHANDLES;

    m_ok = true;
}

Process::~Process()
{
    if (m_pi.hProcess != INVALID_HANDLE_VALUE)
        TerminateProcess(m_pi.hProcess, 0);
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

DWORD Process::RemoteCall(LPTHREAD_START_ROUTINE call, LPVOID arg)
{
    DWORD exitcode = 0;
    DWORD tid;
    HANDLE rth = CreateRemoteThread(m_pi.hProcess, NULL, 0, call, arg, 0, &tid);
    WaitForSingleObject(rth, INFINITE);
    GetExitCodeThread(rth, &exitcode);
    return exitcode;
}

/* scan.exe v1 uses WriteConsoleA for the output, this makes impossible to capture it, so we need to redirect
   WriteConsoleA to WriteFile stdout */

BOOL Process::RedirectConsole(void)
{
    DWORD oldprot, dummy = 0;
    DWORD bread, bwrite;
    IMAGE_DOS_HEADER DosHeader;
    IMAGE_NT_HEADERS NTHeader;
    IMAGE_IMPORT_DESCRIPTOR ImportDesc;
    IMAGE_THUNK_DATA Thunk;
    PROC pfnOriginalProc;
    const char kernel32[] = "kernel32.dll";
    char pszModName[] = "XXXXXXXXXXXX";
    size_t lk32 = strlen(kernel32);
    HMODULE k32 = GetModuleHandleA(kernel32);

    pfnOriginalProc = GetProcAddress(k32, "WriteConsoleA");

    ULONG_PTR base = RemoteCall((LPTHREAD_START_ROUTINE) GetModuleHandleA, NULL);

    if (!base)
        return FALSE;

    if (!ReadProcessMemory(m_pi.hProcess, (LPCVOID) base, &DosHeader, sizeof(DosHeader), &bread))
        return FALSE;

    if (DosHeader.e_magic != IMAGE_DOS_SIGNATURE)
        return FALSE;

    if (!ReadProcessMemory(m_pi.hProcess, (LPCVOID) (base + DosHeader.e_lfanew), &NTHeader, sizeof(NTHeader), &bread))
        return FALSE;

    if (NTHeader.Signature != IMAGE_NT_SIGNATURE)
        return FALSE;

    UINT_PTR impdesc = base + NTHeader.OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress;

    do
    {
        if (!ReadProcessMemory(m_pi.hProcess, (LPCVOID) impdesc, &ImportDesc, sizeof(ImportDesc), &bread))
            return FALSE;
        if (!ReadProcessMemory(m_pi.hProcess, (LPCVOID) (base + ImportDesc.Name), pszModName, lk32, &bread))
            return FALSE;
        if (_stricmp(pszModName, kernel32) == 0)
            break;
        impdesc += sizeof(ImportDesc);
    } while (ImportDesc.Name);

    if (!ImportDesc.Name)
        return FALSE;

    UINT_PTR thunk = base + ImportDesc.FirstThunk;

    do
    {
        if (!ReadProcessMemory(m_pi.hProcess, (LPCVOID) thunk, &Thunk, sizeof(Thunk), &bread))
            return FALSE;

        if (Thunk.u1.Function == (LONG_PTR) pfnOriginalProc)
        {
            DWORD rel;
            unsigned char code[] = {
                0x6a, 0xf5,                     /* push STD_OUTPUT_HANDLE */
                0xe8, 0x00, 0x00, 0x00, 0x00,   /* call GetStdhandle */
                0x89, 0x44, 0x24, 0x04,         /* mov [esp + 4], eax */
                0xe9, 0x00, 0x00, 0x00, 0x00    /* jmp WriteFile */
            };
            DWORD rcode = (ULONG_PTR) VirtualAllocEx(m_pi.hProcess, NULL, sizeof(code), MEM_RESERVE | MEM_COMMIT, PAGE_EXECUTE_READWRITE);

            rel = (DWORD) GetProcAddress(k32, "GetStdHandle") - (rcode + 2) - 5;
            memcpy(&code[3], &rel, sizeof(DWORD));

            rel = (DWORD) GetProcAddress(k32, "WriteFile") - (rcode + 11) - 5;
            memcpy(&code[12], &rel, sizeof(DWORD));

            if (!WriteProcessMemory(m_pi.hProcess, (LPVOID) rcode, code, sizeof(code), &bwrite))
                return FALSE;

            if (!VirtualProtectEx(m_pi.hProcess, (LPVOID) thunk, sizeof(DWORD), PAGE_EXECUTE_READWRITE, &oldprot))
                return FALSE;

            if (!WriteProcessMemory(m_pi.hProcess, (LPVOID) thunk, &rcode, sizeof(DWORD), &bwrite))
                return FALSE;

            VirtualProtectEx(m_pi.hProcess, (LPVOID) thunk, sizeof(DWORD), oldprot, &dummy);

            return TRUE;
        }
        thunk += sizeof(Thunk);
    }
    while (Thunk.u1.Function);

    return TRUE;
}

DWORD Process::Exec(WTL::CString &result)
{
    DWORD exitcode = -1;
    DWORD dwThreadId;
    HANDLE hThread;

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

    RedirectConsole();
    hThread = CreateThread(NULL, 0, OutputThread, (LPVOID) this, 0, &dwThreadId);

    if (WaitForSingleObject(hThread, INFINITE) == WAIT_OBJECT_0)
    {
        if (GetExitCodeProcess(m_pi.hProcess, &exitcode))
        {
            CloseHandle(m_pi.hThread);
            CloseHandle(m_pi.hProcess);
            CloseHandle(m_child);
            CloseHandle(m_pipe);
            m_pi.hProcess = INVALID_HANDLE_VALUE;
            result = m_buffer;
            return exitcode;
        }
    }

    return -1;
}
