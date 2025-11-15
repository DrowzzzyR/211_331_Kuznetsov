#ifndef INTEGRITYCHECK_H
#define INTEGRITYCHECK_H

#include <windows.h>
#include <QtCore/QtGlobal>
#include <QByteArray>

// Класс для самопроверки контрольной суммы части приложения в виртуальной памяти
// Вычисляет SHA-256 хеш сегмента .text и сравнивает с эталонным значением
class IntegrityCheck
{
public:
    // Проверка целостности сегмента .text
    // Вычисляет текущий хеш и сравнивает с эталонным
    static bool verifyTextSegment();
    
    // Вычисление SHA-256 хеша сегмента .text
    // Возвращает хеш в формате QByteArray (32 байта)
    static QByteArray calculateTextSegmentHash();
    
    // Получение эталонного хеша
    // В разработке возвращает пустой массив, в финальной версии должен содержать реальный хеш
    static QByteArray getExpectedHash();

private:
    // Получение информации о сегменте .text в PE-файле
    // Возвращает адрес и размер сегмента в виртуальной памяти
    static bool getTextSegmentInfo(PVOID &baseAddress, SIZE_T &size);
};

#endif

