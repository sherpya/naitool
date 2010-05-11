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
#include "zfile.h"

ZFile::ZFile(const char *path)
{
    m_uf = unzOpen(path);
}

ZFile::~ZFile()
{
    unzCloseCurrentFile(m_uf);
    unzClose(m_uf);
}

BOOL ZFile::Reset(void)
{
    if (unzGoToFirstFile(m_uf) != UNZ_OK)
        return FALSE;
    return GetCurrentFileInfo();    
}

BOOL ZFile::Next(void)
{
    if (unzGoToNextFile(m_uf) != UNZ_OK)
        return FALSE;
    return GetCurrentFileInfo();
}

const char *ZFile::GetCurrentFileName(void)
{
    return const_cast<const char *>(m_cfilename);
}

BOOL ZFile::GetCurrentFileInfo(void)
{
    return (unzGetCurrentFileInfo(m_uf, &m_finfo, m_cfilename, sizeof(m_cfilename), NULL, 0, NULL, 0) == UNZ_OK);
}

BOOL ZFile::Unzip(const wchar_t *destination)
{
    if (unzOpenCurrentFile(m_uf) != UNZ_OK)
        return FALSE;

    HANDLE hOut = CreateFile(destination, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hOut == INVALID_HANDLE_VALUE)
        return FALSE;

    BYTE buffer[8192];
    DWORD bwrite;
    int nRet;
    do
    {
        nRet = unzReadCurrentFile(m_uf, buffer, sizeof(buffer));
        if (nRet <= 0) break;
        if (!WriteFile(hOut, buffer, nRet, &bwrite, NULL) || (bwrite != nRet))
        {
            nRet = UNZ_ERRNO;
            break;
        }
    }
    while (nRet > 0);

    CloseHandle(hOut);
    unzCloseCurrentFile(m_uf);

    return (nRet == UNZ_OK);
}
