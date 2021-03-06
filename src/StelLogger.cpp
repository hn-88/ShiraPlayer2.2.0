/*
 * ShiraPlayer(TM)
 * Copyright (C) 2002 Fabien Chereau
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
#include "StelLogger.hpp"
#include "StelUtils.hpp"

#include <config.h>
#include <QDateTime>
#include <QProcess>
#ifdef Q_OS_WIN
#include <windows.h>
#endif

// Init statics variables.
QFile StelLogger::logFile;
QString StelLogger::log;

void StelLogger::init(const QString& logFilePath)
{
    logFile.setFileName(logFilePath);

    if (logFile.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text | QIODevice::Unbuffered))
        qInstallMessageHandler(StelLogger::debugLogHandler);

    // write timestamp
    writeLog(QString("%1").arg(QDateTime::currentDateTime().toString(Qt::ISODate)));

    // write OS version
    writeLog(StelUtils::getOperatingSystemInfo());

    // write GCC version
#ifndef __GNUC__
    writeLog("Non-GCC compiler");
#else
    writeLog(QString("Compiled with GCC %1.%2.%3").arg(__GNUC__).arg(__GNUC_MINOR__).arg(__GNUC_PATCHLEVEL__));
#endif

    // write Qt version
    writeLog(QString("Qt runtime version: %1").arg(qVersion()));
    writeLog(QString("Qt compilation version: %1").arg(QT_VERSION_STR));

    // write addressing mode
//#ifdef __LP64__
#if defined(__LP64__) || defined(_WIN64)
    writeLog("Addressing mode: 64-bit");
#else
    writeLog("Addressing mode: 32-bit");
#endif

    // write memory and CPU info
#ifdef Q_OS_LINUX

#ifndef USE_OPENGL_ES2
    QFile infoFile("/proc/meminfo");
    if(!infoFile.open(QIODevice::ReadOnly | QIODevice::Text))
        writeLog("Could not get memory info.");
    else
    {
        while(!infoFile.peek(1).isEmpty())
        {
            QString line = infoFile.readLine();
            line.chop(1);
            if (line.startsWith("Mem") || line.startsWith("SwapTotal"))
                writeLog(line);
        }
        infoFile.close();
    }

    infoFile.setFileName("/proc/cpuinfo");
    if (!infoFile.open(QIODevice::ReadOnly | QIODevice::Text))
        writeLog("Could not get CPU info.");
    else
    {
        while(!infoFile.peek(1).isEmpty())
        {
            QString line = infoFile.readLine();
            line.chop(1);
            if(line.startsWith("model name") || line.startsWith("cpu MHz"))
                writeLog(line);
        }
        infoFile.close();
    }

    QProcess lspci;
    lspci.start("lspci -v", QIODevice::ReadOnly);
    lspci.waitForFinished(200);
    const QString pciData(lspci.readAll());
    QStringList pciLines = pciData.split('\n', QString::SkipEmptyParts);
    for (int i = 0; i<pciLines.size(); i++)
    {
        if(pciLines.at(i).contains("VGA compatible controller"))
        {
            writeLog(pciLines.at(i));
            i++;
            while(i < pciLines.size() && pciLines.at(i).startsWith('\t'))
            {
                if(pciLines.at(i).contains("Kernel driver in use"))
                    writeLog(pciLines.at(i).trimmed());
                else if(pciLines.at(i).contains("Kernel modules"))
                    writeLog(pciLines.at(i).trimmed());
                i++;
            }
        }
    }
#endif // USE_OPENGL_ES2

// Aargh Windows API
#elif defined Q_OS_WIN
    // Hopefully doesn't throw a linker error on earlier systems. Not like
    // I'm gonna test it or anything.
    if (QSysInfo::WindowsVersion >= QSysInfo::WV_2000)
    {
#ifdef __LP64__
        MEMORYSTATUSEX statex;
        GlobalMemoryStatusEx(&statex);
        writeLog(QString("Total physical memory: %1 MB (unreliable)").arg(statex.ullTotalPhys/(1024<<10)));
        writeLog(QString("Total virtual memory: %1 MB (unreliable)").arg(statex.ullTotalVirtual/(1024<<10)));
        writeLog(QString("Physical memory in use: %1%").arg(statex.dwMemoryLoad));
#else
        MEMORYSTATUS statex;
        GlobalMemoryStatus(&statex);
        writeLog(QString("Total memory: %1 MB (unreliable)").arg(statex.dwTotalPhys/(1024<<10)));
        writeLog(QString("Total virtual memory: %1 MB (unreliable)").arg(statex.dwTotalVirtual/(1024<<10)));
        writeLog(QString("Physical memory in use: %1%").arg(statex.dwMemoryLoad));
#endif
    }
    else
        writeLog("Windows version too old to get memory info.");

    HKEY hKey = NULL;
    DWORD dwType = REG_DWORD;
    DWORD numVal = 0;
    DWORD dwSize = sizeof(numVal);

    // iterate over the processors listed in the registry
    QString procKey = "Hardware\\Description\\System\\CentralProcessor";
    LONG lRet = ERROR_SUCCESS;
    int i;
    for(i = 0; lRet == ERROR_SUCCESS; i++)
    {
        lRet = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                            TEXT(qPrintable(QString("%1\\%2").arg(procKey).arg(i))),
                            0, KEY_QUERY_VALUE, &hKey);

        if(lRet == ERROR_SUCCESS)
        {
            if(RegQueryValueEx(hKey, "~MHz", NULL, &dwType, (LPBYTE)&numVal, &dwSize) == ERROR_SUCCESS)
                writeLog(QString("Processor speed: %1 MHz").arg(numVal));
            else
                writeLog("Could not get processor speed.");
        }

        // can you believe this trash?
        dwType = REG_SZ;
        char nameStr[512];
        DWORD nameSize = sizeof(nameStr);

        if (lRet == ERROR_SUCCESS)
        {
            if (RegQueryValueEx(hKey, "ProcessorNameString", NULL, &dwType, (LPBYTE)&nameStr, &nameSize) == ERROR_SUCCESS)
                writeLog(QString("Processor name: %1").arg(nameStr));
            else
                writeLog("Could not get processor name.");
        }

        RegCloseKey(hKey);
    }
    if(i == 0)
        writeLog("Could not get processor info.");

#elif defined Q_OS_MAC
    writeLog("You look like a Mac user. How would you like to write some system info code here? That would help a lot.");

#endif
}

void StelLogger::deinit()
{
    qInstallMessageHandler(0);
    logFile.close();
}

void StelLogger::debugLogHandler(QtMsgType, const QMessageLogContext&, const QString& msg)
{
    fprintf(stderr, "%s\n", msg.toUtf8().constData());
    writeLog(QString(msg));
}

void StelLogger::writeLog(QString msg)
{
    msg += "\n";
    logFile.write(qPrintable(msg), msg.size());
    log += msg;
}
