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
