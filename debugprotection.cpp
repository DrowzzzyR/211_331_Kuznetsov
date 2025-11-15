#include "debugprotection.h"
#include <windows.h>

#ifdef _WIN64
typedef DWORD64 DWORD_PTR;
#else
typedef DWORD DWORD_PTR;
#endif

// Проверка наличия отладчика через стандартный Windows API
// Возвращает true, если процесс запущен под отладчиком
bool DebugProtection::isDebuggerPresent()
{
    return ::IsDebuggerPresent() != FALSE;
}

// Проверка флага BeingDebugged в Process Environment Block (PEB)
// Использует прямой доступ к PEB через сегментные регистры
// Более надежный метод, так как не использует функции, которые могут быть перехвачены
bool DebugProtection::checkPEB()
{
    PPEB peb = nullptr;
    
    // Получаем указатель на PEB через сегментный регистр
    // Для x64 используется GS регистр, для x86 - FS регистр
#ifdef _WIN64
    peb = (PPEB)__readgsqword(0x60);  // x64: GS[0x60] указывает на PEB
#else
    peb = (PPEB)__readfsdword(0x30); // x86: FS[0x30] указывает на PEB
#endif
    
    // Проверяем флаг BeingDebugged
    if (peb && peb->BeingDebugged) {
        return true;
    }
    
    return false;
}

// Проверка наличия DebugPort через низкоуровневый API NtQueryInformationProcess
// DebugPort - это порт отладки процесса, который ненулевой, если процесс отлаживается
bool DebugProtection::checkNtQuery()
{
    // Определение типа функции NtQueryInformationProcess
    typedef NTSTATUS (WINAPI *pfnNtQueryInformationProcess)(
        HANDLE ProcessHandle,
        DWORD ProcessInformationClass,
        PVOID ProcessInformation,
        ULONG ProcessInformationLength,
        PULONG ReturnLength
    );
    
    // Получаем дескриптор модуля ntdll.dll
    HMODULE hNtdll = GetModuleHandleA("ntdll.dll");
    if (!hNtdll) {
        return false;
    }
    
    // Получаем адрес функции NtQueryInformationProcess
    pfnNtQueryInformationProcess NtQueryInformationProcess = 
        (pfnNtQueryInformationProcess)GetProcAddress(hNtdll, "NtQueryInformationProcess");
    
    if (!NtQueryInformationProcess) {
        return false;
    }
    
    // Запрашиваем информацию о DebugPort (класс 7 = ProcessDebugPort)
    DWORD_PTR debugPort = 0;
    NTSTATUS status = NtQueryInformationProcess(
        GetCurrentProcess(),
        7,  // ProcessDebugPort
        &debugPort,
        sizeof(debugPort),
        nullptr
    );
    
    // Если status == 0 (успех) и debugPort != 0, значит процесс отлаживается
    return (status == 0 && debugPort != 0);
}

