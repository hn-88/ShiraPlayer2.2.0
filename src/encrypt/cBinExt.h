/*
 * ShiraPlayer(TM)
 * Copyright (C) 2011 Asaf Yurdakul
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 * ShiraPlayer is a trademark of Sureyyasoft.
 */

#ifndef CBINEXT_H
#define CBINEXT_H

#include "hash_defines.h"
#include <QMessageBox>

#ifdef WIN32
#include <windows.h>
#endif

#include <fstream>
#include <sstream>
#include <map>

//zlib compression
#include "zlib.h"
#include "blowfish/blowfish.h"
//#include "cselfextractor.h"

//class cselfextractor;


class cBinExt
{

private:
    //Properties ---------------------------------
    /**
      all the files in this list will be compressed.
    */
    StringList m_FileList;

    /**
      Index section of the self extracting binary will be of this size (kb)
      if data comes which exceeds this size, the compression will fail
    */
    const unsigned long m_IndexSize;     //in bytes

    /**
      The current binnary will be buffered in this one time
    */
    unsigned char *m_ucBinBuffer;

    /**
      This will contain the index of the attachments, if attachments are found in the application
    */
    unsigned char *m_ucBinIndex;

    /**
      Size of @m_ucBinBuffer
    */
    unsigned long m_BinBufferSize;

    /**
      Full path of the binary running
    */
    std::string m_BinPath;

    /**
      if some error occured during the construction of the object, this will be true
    */
    bool m_bObjectCorrupt;



protected:
    //Methods ------------------------------------------


    /**
      Reads the index from the compress binary and fills into <path,size> pair
    */
    void ReadIndexFromBinary(std::vector<std::string>& files, std::vector<std::string>& sizes);

    /**
      Encrypts the @Pass string and later stores in the binary
    */
    short Encrypt(std::string& Pass, std::vector<unsigned long>& keys);

    /**
      @Pass and @keys are pass to the function, if they match with the string it returns SUCCESS else FAIL
    */
    short Decrypt(std::string& Pass, std::vector<unsigned long*>& keys);

    /**
      Reads the encryption keys from the binary which were inserted during compression process
    */
    short ReadEncryptionKeys(std::vector<unsigned long *>& keys);

public:
    cBinExt();
    ~cBinExt();

     /**
      If attachments are present which can be found calling IsAttachmentPresent()
      then all the files will be extracted to @OutputPath
      compression and de-compression of the data is done using the zlib in-memory compression library.
      see uncompr.c and visit http://zlib.net

      @Pass contains the password for the archive which will be used for decryption of the keys strored in the binary
      @object will contain the cselfextractor object to update the progress bar
    */
    short ExtractFiles(const std::string InputFile,const std::string OutputPath, std::string Pass);

    /**
      See if something is attached in me
      if something is attached return true, and which means i have to extract from me
      Set @m_ucBinIndex buffer to the start of last @m_IndexSize bytes. It will be reffered to when extracting items
      return SUCCESS if attachment found else INX_NOT_FOUND if not found, in other cases error number is returned.
    */
    short IsAttachmentPresent();

    enum MSG_TYPE
    {
        ERR = 0,
        WARN = 1,
        INFO = 2
    };

    /**
      Shows a message box with the text and @Type
    */
    void ShowMessage(std::string Message, MSG_TYPE Type = INFO);


    /**
      Creates a compressed file from the file list @m_FileList
      this will only happen when its confirmed that there are no attachments present in which case extraction will be done.
      the compressed file will be created at @OutputPath which should exist

      File Structure (will help if we start from end):
         _______________
        |Executable data|
        |_______________|
        |Embeded file(s)|
        |_______________|___________________________________
        |encrypted keys, size = 2 * sizeof(unsigned long)   |
        |___________________________________________________|
        |Index telling the size and |
        |name of embeded file       |
        |This section will be 100kb |
        |___________________________|
        |Index structure            |
        |*IDX*\n                    |
        |FileName_1 \t size \n      |
        |FileName_2 \t size \n      |
        |FileName_n \t size \n      |
        |*EIDX*\n                   |
        |___________________________|

        Compression of the files is done using zlib library which does the in-memory compression of the data.
        see compress.c and visit http://zlib.net
    */
    short CreateCompressedFile(const std::string InputFile,const std::string OutputPath,  std::string& Pass);
    /**
      Sets/Gets the Binary Path
    */
    bool SetBinPath(std::string path);
    std::string& GetBinPath();

    /**
      this will buffer the whole binary in @m_ucBinBuffer and sets the size of @m_BinBufferSize
    */
    short BufferFile(const std::string& FilePath, unsigned char **ucFileBuffer, unsigned long *FileLength);
    bool  DoBufferFile();

    QPixmap ExtractImage(const std::string InputFile, std::string Pass);
};

#endif // CBINEXT_H
