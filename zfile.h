
#include "zlib/unzip.h"

class ZFile
{
public:
    ZFile(const char *path);
    ~ZFile();
    BOOL Reset(void);
    BOOL Next(void);
    BOOL Unzip(const wchar_t *destination);
    const char *GetCurrentFileName(void);
private:
    BOOL GetCurrentFileInfo(void);

    unzFile m_uf;
    unz_file_info m_finfo;
    char m_cfilename[MAX_PATH];
};
