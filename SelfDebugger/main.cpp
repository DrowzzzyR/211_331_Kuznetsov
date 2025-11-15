#include <windows.h>
#include <iostream>
#include <string>
#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>

#pragma comment(lib, "advapi32.lib")

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    STARTUPINFO si;
    PROCESS_INFORMATION pi;
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));
    
    QString selfDebuggerPath = QCoreApplication::applicationDirPath();
    QDir selfDebuggerDir(selfDebuggerPath);
    
    QString appPath;
    
    QString path1 = selfDebuggerDir.absoluteFilePath("211_331_Kuznetsov.exe");
    if (QFile::exists(path1)) {
        appPath = path1;
    } else {
        selfDebuggerDir.cdUp();
        QString path2 = selfDebuggerDir.absoluteFilePath("211_331_Kuznetsov.exe");
        if (QFile::exists(path2)) {
            appPath = path2;
        } else {
            selfDebuggerDir.cdUp();
            QString path3 = selfDebuggerDir.absoluteFilePath("build/Desktop_Qt_6_10_0_MSVC2022_64bit-Debug/211_331_Kuznetsov.exe");
            if (QFile::exists(path3)) {
                appPath = path3;
            } else {
                std::cout << "Ошибка: Не удалось найти 211_331_Kuznetsov.exe" << std::endl;
                return 1;
            }
        }
    }
    
    std::cout << "Запуск приложения: " << appPath.toStdString() << std::endl;
    
    QByteArray appPathBytes = appPath.toUtf8();
    std::string appPathStr = appPathBytes.toStdString();
    
    int wlen = MultiByteToWideChar(CP_UTF8, 0, appPathStr.c_str(), -1, nullptr, 0);
    wchar_t* cmdLine = new wchar_t[wlen];
    MultiByteToWideChar(CP_UTF8, 0, appPathStr.c_str(), -1, cmdLine, wlen);
    
    si.dwFlags = STARTF_USESHOWWINDOW;
    si.wShowWindow = SW_SHOW;
    
    QString workingDir = QFileInfo(appPath).absolutePath();
    QByteArray workingDirBytes = workingDir.toUtf8();
    std::string workingDirStr = workingDirBytes.toStdString();
    int wlenDir = MultiByteToWideChar(CP_UTF8, 0, workingDirStr.c_str(), -1, nullptr, 0);
    wchar_t* workingDirW = new wchar_t[wlenDir];
    MultiByteToWideChar(CP_UTF8, 0, workingDirStr.c_str(), -1, workingDirW, wlenDir);
    
    if (CreateProcessW(
                NULL,
                cmdLine,
                NULL,
                NULL,
                FALSE,
                CREATE_SUSPENDED,
                NULL,
                workingDirW,
                &si,
                &pi)) {
        std::cout << "Процесс создан, PID: " << pi.dwProcessId << std::endl;
    } else {
        std::cout << "Ошибка CreateProcessW: " << GetLastError() << std::endl;
        delete[] cmdLine;
        delete[] workingDirW;
        return 1;
    }
    
    delete[] cmdLine;
    delete[] workingDirW;
    
    bool isAttached = DebugActiveProcess(pi.dwProcessId);
    if (!isAttached) {
        DWORD lastError = GetLastError();
        std::cout << "Ошибка DebugActiveProcess: " << lastError << std::endl;
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
        return 1;
    } else {
        std::cout << "Подключен как отладчик к процессу" << std::endl;
    }
    
    ResumeThread(pi.hThread);
    
    std::cout << "Ожидание событий отладки..." << std::endl;
    
    DEBUG_EVENT debugEvent;
    bool continueDebugging = true;
    
    while (continueDebugging) {
        bool result1 = WaitForDebugEvent(&debugEvent, INFINITE);
        if (!result1) {
            break;
        }
        
        DWORD continueStatus = DBG_CONTINUE;
        
        switch (debugEvent.dwDebugEventCode) {
            case EXCEPTION_DEBUG_EVENT:
                if (debugEvent.u.Exception.ExceptionRecord.ExceptionCode == EXCEPTION_BREAKPOINT ||
                    debugEvent.u.Exception.ExceptionRecord.ExceptionCode == EXCEPTION_SINGLE_STEP) {
                    continueStatus = DBG_CONTINUE;
                } else {
                    continueStatus = DBG_EXCEPTION_NOT_HANDLED;
                }
                break;
                
            case CREATE_PROCESS_DEBUG_EVENT:
                if (debugEvent.u.CreateProcessInfo.hFile) {
                    CloseHandle(debugEvent.u.CreateProcessInfo.hFile);
                }
                break;
                
            case EXIT_PROCESS_DEBUG_EVENT:
                std::cout << "Процесс завершен, код выхода: " 
                          << debugEvent.u.ExitProcess.dwExitCode << std::endl;
                continueDebugging = false;
                break;
                
            case CREATE_THREAD_DEBUG_EVENT:
                break;
                
            case EXIT_THREAD_DEBUG_EVENT:
                break;
                
            case LOAD_DLL_DEBUG_EVENT:
                if (debugEvent.u.LoadDll.hFile) {
                    CloseHandle(debugEvent.u.LoadDll.hFile);
                }
                break;
                
            case UNLOAD_DLL_DEBUG_EVENT:
                break;
                
            case OUTPUT_DEBUG_STRING_EVENT:
                break;
        }
        
        bool result2 = ContinueDebugEvent(debugEvent.dwProcessId,
                              debugEvent.dwThreadId,
                              continueStatus);
        if (!result2) {
            break;
        }
    }
    
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    
    std::cout << "Отладчик завершен" << std::endl;
    
    return 0;
}

