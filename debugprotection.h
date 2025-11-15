#ifndef DEBUGPROTECTION_H
#define DEBUGPROTECTION_H

#include <windows.h>

// Структура Process Environment Block (PEB)
// Используется для проверки флага отладки напрямую из памяти процесса
typedef struct _PEB {
    BOOLEAN InheritedAddressSpace;
    BOOLEAN ReadImageFileExecOptions;
    BOOLEAN BeingDebugged;  // Флаг, указывающий на наличие отладчика
    union {
        BOOLEAN BitField;
        struct {
            BOOLEAN ImageUsesLargePages : 1;
            BOOLEAN IsProtectedProcess : 1;
            BOOLEAN IsLegacyProcess : 1;
            BOOLEAN IsImageDynamicallyRelocated : 1;
            BOOLEAN SkipPatchingUser32Forwarders : 1;
            BOOLEAN SpareBits : 3;
        };
    };
    HANDLE Mutant;
    PVOID ImageBaseAddress;
} PEB, *PPEB;

// Класс для защиты от отладки приложения
// Реализует три метода обнаружения присоединения отладчика
class DebugProtection
{
public:
    // Проверка наличия отладчика через Windows API IsDebuggerPresent()
    static bool isDebuggerPresent();
    
    // Проверка флага BeingDebugged в PEB напрямую через сегментные регистры
    static bool checkPEB();
    
    // Проверка наличия DebugPort через NtQueryInformationProcess
    static bool checkNtQuery();
};

#endif

