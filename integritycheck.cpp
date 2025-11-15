#include "integritycheck.h"
#include <imagehlp.h>
#include <QCryptographicHash>
#include <QDebug>

#pragma comment(lib, "imagehlp.lib")

bool IntegrityCheck::getTextSegmentInfo(PVOID &baseAddress, SIZE_T &size)
{
    HMODULE hModule = GetModuleHandle(nullptr);
    if (!hModule) {
        return false;
    }
    
    PIMAGE_DOS_HEADER dosHeader = (PIMAGE_DOS_HEADER)hModule;
    if (dosHeader->e_magic != IMAGE_DOS_SIGNATURE) {
        return false;
    }
    
    PIMAGE_NT_HEADERS ntHeaders = (PIMAGE_NT_HEADERS)((BYTE*)hModule + dosHeader->e_lfanew);
    if (ntHeaders->Signature != IMAGE_NT_SIGNATURE) {
        return false;
    }
    
    PIMAGE_SECTION_HEADER sectionHeader = IMAGE_FIRST_SECTION(ntHeaders);
    
    for (int i = 0; i < ntHeaders->FileHeader.NumberOfSections; i++) {
        if (strcmp((char*)sectionHeader[i].Name, ".text") == 0) {
            baseAddress = (PVOID)((BYTE*)hModule + sectionHeader[i].VirtualAddress);
            size = sectionHeader[i].Misc.VirtualSize;
            return true;
        }
    }
    
    return false;
}

QByteArray IntegrityCheck::calculateTextSegmentHash()
{
    PVOID baseAddress = nullptr;
    SIZE_T size = 0;
    
    if (!getTextSegmentInfo(baseAddress, size)) {
        return QByteArray();
    }
    
    QCryptographicHash hash(QCryptographicHash::Sha256);
    QByteArray segmentData = QByteArray::fromRawData(
        reinterpret_cast<const char*>(baseAddress), 
        static_cast<int>(size)
    );
    hash.addData(segmentData);
    
    return hash.result();
}

QByteArray IntegrityCheck::getExpectedHash()
{
    return QByteArray();
}

bool IntegrityCheck::verifyTextSegment()
{
    QByteArray expectedHash = getExpectedHash();
    
    if (expectedHash.isEmpty()) {
        qDebug() << "IntegrityCheck: эталонный хеш не установлен, проверка пропущена";
        #ifndef _DEBUG
        #endif
        return true;
    }
    
    QByteArray calculatedHash = calculateTextSegmentHash();
    
    if (calculatedHash.isEmpty()) {
        qDebug() << "IntegrityCheck: не удалось вычислить хеш сегмента .text";
        return false;
    }
    
    bool isValid = (calculatedHash == expectedHash);
    
    if (!isValid) {
        qDebug() << "IntegrityCheck: хеши не совпадают!";
        qDebug() << "Ожидаемый (Base64):" << expectedHash.toBase64();
        qDebug() << "Вычисленный (Base64):" << calculatedHash.toBase64();
    } else {
        qDebug() << "IntegrityCheck: проверка целостности пройдена успешно";
    }
    
    return isValid;
}

