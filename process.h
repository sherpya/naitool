class Process
{
public:
    Process(WTL::CString cmdline);
    DWORD Exec(WTL::CString &result);

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
