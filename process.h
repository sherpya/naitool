class Process
{
public:
    Process(WTL::CString cmdline);
    DWORD Exec(WTL::CString &result);
    BOOL Prepare(void);

    static DWORD WINAPI OutputThread(LPVOID lpvThreadParam);

private:
    WTL::CString m_cmdline;
    WTL::CString m_buffer;

    HANDLE m_hStdoutR;
    HANDLE m_hStdoutW;
    HANDLE m_hStderrW;
    HANDLE m_hStdoutRDup;

    HANDLE m_hEvtStop;

    SECURITY_ATTRIBUTES m_saAttr;
    PROCESS_INFORMATION m_pi;
    STARTUPINFO m_si;
};

