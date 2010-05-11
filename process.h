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

class Process
{
public:
    Process(WTL::CString cmdline);
    ~Process();
    DWORD Exec(WTL::CString &result);
    DWORD Process::RemoteCall(LPTHREAD_START_ROUTINE call, LPVOID arg);
    BOOL RedirectConsole(void);

    static DWORD WINAPI OutputThread(LPVOID lpvThreadParam);

private:
    WTL::CString m_cmdline;
    WTL::CString m_buffer;

    bool m_ok;
    HANDLE m_pipe, m_child;

    SECURITY_ATTRIBUTES m_saAttr;
    PROCESS_INFORMATION m_pi;
    STARTUPINFO m_si;
};
