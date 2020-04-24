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
#include "cBinExt.h"

#include "QDebug"

/**
  constructor
*/
cBinExt::cBinExt() :
        m_IndexSize(100 * 1024),     //100kb
        m_ucBinBuffer(NULL),
        m_BinBufferSize(0),
        m_BinPath(""),
        m_bObjectCorrupt(false)
{

}

cBinExt::~cBinExt()
{
    if(m_ucBinBuffer != NULL)
    {
        delete [] m_ucBinBuffer;
    }

}



short cBinExt::IsAttachmentPresent()
{
    if(m_bObjectCorrupt)
    {
        ShowMessage("Application cannot be created properly. Restart me and try again.",ERR);
        return FAIL;
    }

    //contains the pointer from the last @m_IndexSize bytes
    m_ucBinIndex = (m_ucBinBuffer + m_BinBufferSize - m_IndexSize);
    if(!strncmp((char*)m_ucBinIndex,"*IDX*",5))
    {
        return SUCCESS;
    }
    else
    {
        return INX_NOT_FOUND;       //which means, it has no attachement to extract
    }
}

bool cBinExt::DoBufferFile()
{
    short bError = SUCCESS;
    if(BufferFile(GetBinPath(), &m_ucBinBuffer, &m_BinBufferSize) != SUCCESS)
    {
        m_bObjectCorrupt = true;
        return false;
    }

    return bError;
}

bool cBinExt::SetBinPath(std::string path)
{
    short bError = SUCCESS;

    m_BinPath = path;

    return bError;
}

inline std::string& cBinExt::GetBinPath()
{
    return m_BinPath;
}

short cBinExt::BufferFile(const std::string& FilePath, unsigned char **ucFileBuffer, unsigned long *FileLength)
{
    short bError = SUCCESS;

    //read the full file first
    std::ifstream ifs(FilePath.c_str(),std::ios::binary);



    if(ifs.fail())
    {
        bError = FILE_OPEN_ERR;
        std::string err = FilePath + " cannot be opened for reading.";
        ShowMessage(err,ERR);
        return false;
    }

    //calculate the length of the file to be returned.
    ifs.seekg(0, std::ios::end);
    *FileLength = ifs.tellg();
    ifs.seekg (0, std::ios::beg);

    *ucFileBuffer = new unsigned char[*FileLength];

    //read the full file
    ifs.read((char*)*ucFileBuffer, *FileLength);
    if(!ifs.gcount())
    {
        delete [] *ucFileBuffer;
        *ucFileBuffer = NULL;
        bError = FAIL;
        return false;
    }



    ifs.close();
    return bError;
}

short cBinExt::CreateCompressedFile(const std::string InputFile,const std::string OutputPath, std::string& Pass)
{
    short bError = SUCCESS;


    //create an output file at @OutputPath
    std::ofstream ofs(OutputPath.c_str(), std::ios::binary);


    std::vector<unsigned long> keys;

    if((bError = Encrypt(Pass, keys)) != SUCCESS)
    {
        return false;
    }

    if(ofs.fail())
    {
        ShowMessage("I cannot create an output file. Check the permissions on the path and try again.", WARN);
        bError = FILE_OPEN_ERR;
        return false;
    }

    //read each attachment (m_FileList) and attach to ofs and then in the end attach the file index.
    unsigned char *ucAttachmentBuffer;
    unsigned long AttachmentLength;
    std::string	AttachmentIndex= "";

    //index start hint
    AttachmentIndex += "*IDX*\n";


    if(BufferFile(GetBinPath(),&ucAttachmentBuffer, &AttachmentLength) != SUCCESS)
    {

    }

    //make a buffer to get the compressed data and send it for compression
    unsigned char *ucCompressedBuffer;
    unsigned long CompressSize;

    CompressSize = compressBound(AttachmentLength);
    ucCompressedBuffer = new unsigned char [CompressSize];


    //compress it
    if(compress2(ucCompressedBuffer, &CompressSize, ucAttachmentBuffer, AttachmentLength, 4) != Z_OK)
    {

        delete [] ucCompressedBuffer;
        delete [] ucAttachmentBuffer;
    }

    //write to the output archive
    ofs.write((char*)ucCompressedBuffer, CompressSize);
    if(ofs.fail())
    {
        //write failed, let the user try again.
        delete [] ucCompressedBuffer;
        delete [] ucAttachmentBuffer;
        ofs.close();
        bError = FAIL;
    }
    //update the index, this will be written at the end of the output file (.exe)
    AttachmentIndex += InputFile;
    AttachmentIndex += "\t";


    std::stringstream ss;
    //during extraction, we will need both sizes, the exact size and the compressed size
    ss << AttachmentLength;
    ss << "|";
    ss << CompressSize;
    AttachmentIndex += ss.str();
    AttachmentIndex += "\n";


    delete [] ucAttachmentBuffer;
    delete [] ucCompressedBuffer;
    ucAttachmentBuffer = NULL;
    ucCompressedBuffer = NULL;

    //index end hint.
    AttachmentIndex += "*EIDX*";

    //now write the whole index, write a total of m_IndexSize bytes (102400).
    //during extraction the program will read this much bytes from the end to understand the index of file attached.

    //encrypted keys, will be matched during extraction
    ofs.write((char*)&keys[0],sizeof(keys[0]));
    ofs.write((char*)&keys[1],sizeof(keys[1]));

#ifdef WIN32
    ofs.write(AttachmentIndex.c_str(), AttachmentIndex.length()); //write data
    ofs.write("", m_IndexSize - AttachmentIndex.length());    //whatever length is extra, write zero's for that
#elif defined(Q_OS_MAC)
    ofs.write(AttachmentIndex.c_str(), m_IndexSize);    //this code is failing on windows
#elif solaris

    //no code here
#endif

    if(ofs.fail())
    {
        //oops all the work gone wrong, I won't try to recover from this error
        bError = IO_ERR;
    }

    ofs.close();    //we are done.


    return bError;
}


QPixmap cBinExt::ExtractImage(const std::string InputFile, std::string Pass)
{
    short bError = SUCCESS;
    QPixmap result = QPixmap();

    std::vector<std::string> files;
    std::vector<std::string>   sizes;

    //Get the index from the binary to extract all the files.
    ReadIndexFromBinary(files, sizes);
    if(files.size() == 0)
    {
        ShowMessage("No files found in archive.",INFO);
        return result;
    }


    //read the encrypted keys, which will be before m_IndexSize bytes
    std::vector<unsigned long *> keys;
    ReadEncryptionKeys(keys);
    //QMessageBox::information(0,"",QString("%0").arg(keys.size(),0,0));
   // ShowMessage(keys.size(),INFO);
    //qDebug()<< "Keys size: "+ QString("%0").arg(keys[0]);
    //qDebug()<< "Keyler" << &keys[0] << "-" << &keys[1];

    //send the keys and the password for matching and then proceed
    if((bError = Decrypt(Pass, keys)) != SUCCESS)
    {
        ShowMessage("Incorrect password supplied.", ERR);
        return result;
    }


    //we have the list, start extracting but from the last. why?? ..... ???
    //Total size will start from 2 * (unsigned long/unsigned char)
    //this is for the two unsigned long's encryption keys written before the index
    //the size will not be unsigned long's but unsigned char's
    unsigned long TotalSize = 2 * (sizeof(unsigned long)/sizeof(unsigned char));
    //m_dialog->setProgressSize(files.size());

    for(int i = files.size() - 1; i >= 0; i--)
    {
        //orignal size for creating the buffer and compressed size to read the amount of data from the binary
        unsigned long lOrigSize, lCommSize;
        lOrigSize = atol(sizes[i].substr(0, sizes[i].find("|")).c_str());
        sizes[i].erase(0,sizes[i].find("|") + 1);
        lCommSize = atol(sizes[i].c_str());
        TotalSize += lCommSize;


        //   try
        // {
        unsigned char *ucUncompressedFileBuff = new unsigned char[lOrigSize];
        //uncompress the data and then write
        if(uncompress(ucUncompressedFileBuff, &lOrigSize,
                      (m_ucBinBuffer + m_BinBufferSize - m_IndexSize - TotalSize),lCommSize) != Z_OK)
        {
            delete [] ucUncompressedFileBuff;
            std::string err = files[i] + " -- cannot be extracted. Continuing with other files.";
            ShowMessage(err, ERR);
            continue;
        }


        if (!result.loadFromData((const uchar*)ucUncompressedFileBuff, (uint)lOrigSize,0,Qt::AutoColor))
        {
            std::string err = files[i] ;
            ShowMessage(err, ERR);
            continue;
        }

        delete [] ucUncompressedFileBuff;
        ucUncompressedFileBuff = NULL;

    }

    return result;

}

short cBinExt::ExtractFiles(const std::string InputFile,const std::string OutputPath, std::string Pass)
{
    short bError = SUCCESS;

    std::vector<std::string> files;
    std::vector<std::string>   sizes;

    //Get the index from the binary to extract all the files.
    ReadIndexFromBinary(files, sizes);
    if(files.size() == 0)
    {
        ShowMessage("No files found in archive.",INFO);
        return false;
    }

    //read the encrypted keys, which will be before m_IndexSize bytes
    std::vector<unsigned long *> keys;
    ReadEncryptionKeys(keys);

    //send the keys and the password for matching and then proceed
    if((bError = Decrypt(Pass, keys)) != SUCCESS)
    {
        ShowMessage("Incorrect password supplied.", ERR);
        return false;
    }


    //we have the list, start extracting but from the last. why?? ..... ???
    //Total size will start from 2 * (unsigned long/unsigned char)
    //this is for the two unsigned long's encryption keys written before the index
    //the size will not be unsigned long's but unsigned char's
    unsigned long TotalSize = 2 * (sizeof(unsigned long)/sizeof(unsigned char));
    //m_dialog->setProgressSize(files.size());

    for(int i = files.size() - 1; i >= 0; i--)
    {
        std::string temp = OutputPath;

        //add path seperator
#ifdef WIN32
        temp += "\\";
#elif defined(Q_OS_MAC)
        temp += "/";
#elif unix
        temp += "/";
#endif

        //read the data from the binary and output it to the file, simple!! :)
        //add file name and complete the full output path
        temp += files[i];

        //orignal size for creating the buffer and compressed size to read the amount of data from the binary
        unsigned long lOrigSize, lCommSize;
        lOrigSize = atol(sizes[i].substr(0, sizes[i].find("|")).c_str());
        sizes[i].erase(0,sizes[i].find("|") + 1);
        lCommSize = atol(sizes[i].c_str());
        TotalSize += lCommSize;


        //   try
        // {
        unsigned char *ucUncompressedFileBuff = new unsigned char[lOrigSize];
        //uncompress the data and then write
        if(uncompress(ucUncompressedFileBuff, &lOrigSize,
                      (m_ucBinBuffer + m_BinBufferSize - m_IndexSize - TotalSize),lCommSize) != Z_OK)
        {
            delete [] ucUncompressedFileBuff;
            std::string err = files[i] + " -- cannot be extracted. Continuing with other files.";
            ShowMessage(err, ERR);
            continue;
        }


        std::ofstream ofs(temp.c_str(), std::ios::binary);
        ofs.write((char*)ucUncompressedFileBuff, lOrigSize);
        ofs.close();

        delete [] ucUncompressedFileBuff;
        ucUncompressedFileBuff = NULL;

    }

    return bError;
}

void cBinExt::ReadIndexFromBinary(std::vector<std::string>& files, std::vector<std::string>& sizes)
{
    //after 6 characters, the index will start,
    //keep on  getting file name and size, accordingly keep on extracting the data from the binary
    std::string str((char*)(m_ucBinIndex + strlen("*IDX*\n")));


    int len;

    while(1)
    {
        //take out filename from path
        std::string filename = str.substr(0,(len=str.find("\t")));
        str.erase(0,len+1);
#ifdef WIN32
        filename = filename.substr(filename.rfind("/")+1);  //path seperators on all the OS is same in QT
#elif defined(Q_OS_MAC)
        filename = filename.substr(filename.rfind("/")+1);
#elif unix

#endif

        //Index.insert(std::pair<std::string,long>(filename, length));
        files.push_back(filename);
        sizes.push_back(str.substr(0,(len = str.find("\n"))).c_str());

        str.erase(0,len+1);

        if(!str.compare(str.substr(0,strlen("*EIDX*")).c_str()))
            break;
    }

}

short cBinExt::Encrypt(std::string& Pass, std::vector<unsigned long>& keys)
{
    unsigned long L;
    unsigned long R;
    BLOWFISH_CTX ctx;
    short bError = SUCCESS;


    do
    {
        Blowfish_Init (&ctx, (unsigned char*)Pass.c_str(), Pass.length());
        R = Pass[1] - '0';
        L = Pass[Pass.length() - 2] - '0';      //convert to ascii value

        Blowfish_Encrypt(&ctx, &L, &R);
        keys.push_back(L);
        keys.push_back(R);

    }while(0);

    return bError;
}

short cBinExt::Decrypt(std::string& Pass, std::vector<unsigned long *>& keys)
{
    unsigned long L;
    unsigned long R;
    BLOWFISH_CTX ctx;
    short bError = SUCCESS;


    do
    {
        Blowfish_Init (&ctx, (unsigned char*)Pass.c_str(), Pass.length());
        R = *keys[0];
        L = *keys[1];
        Blowfish_Decrypt(&ctx, &L, &R);
        //qDebug()<< "Keyler" << &L << "-" << &R;

        if((Pass[1] - '0' == R) && (L == Pass[Pass.length() - 2] - '0'))
        {
            //success
        }
        else
        {
            bError = FAIL;
        }
    }while(0);

    return bError;
}

void cBinExt::ShowMessage(std::string Message, MSG_TYPE Type)
{
    QMessageBox::Icon i;
    QString title;

    switch (Type)
    {
    case ERR:
        i = QMessageBox::Critical;
        title = "Error";
        break;
    case WARN:
        i = QMessageBox::Warning;
        title = "Warning";
        break;
    case INFO:
        i = QMessageBox::Information;
        title = "Information";
        break;    
    }

    QMessageBox msg(i, title, Message.c_str());
    msg.exec();

}

short cBinExt::ReadEncryptionKeys(std::vector<unsigned long *>& keys)
{
    void *ptr = m_ucBinBuffer + m_BinBufferSize - m_IndexSize;
    unsigned long *ptrLng = (unsigned long*)ptr;


    keys.push_back((ptrLng - 1));
    keys.push_back((ptrLng - 2));

    return SUCCESS;
}
