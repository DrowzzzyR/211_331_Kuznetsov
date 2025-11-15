#ifndef ENCRYPTIONMANAGER_H
#define ENCRYPTIONMANAGER_H

#include <QString>
#include <QByteArray>

/// Класс для шифрования и расшифровки данных с использованием AES-256-CBC
class EncryptionManager
{
public:
    EncryptionManager();
    
    /// Загружает ключ шифрования из файла (поддерживает base64 и hex)
    bool loadKeyFromFile(const QString &filePath, QString &errorMessage);
    
    /// Устанавливает ключ шифрования напрямую (32 байта)
    bool setKey(const QByteArray &key);
    
    /// Проверяет, готов ли менеджер к работе (ключ загружен)
    bool isReady() const;
    
    /// Шифрует данные, возвращает IV + зашифрованные данные
    QByteArray encrypt(const QByteArray &plainData, QString &errorMessage) const;
    
    /// Расшифровывает данные (ожидает IV + зашифрованные данные)
    QByteArray decrypt(const QByteArray &encryptedData, QString &errorMessage) const;

private:
    static const int KEY_SIZE = 32;  // Размер ключа AES-256 (32 байта)
    static const int IV_SIZE = 16;   // Размер вектора инициализации (16 байт)
    
    /// Декодирует ключ из различных форматов (base64, hex, raw)
    static QByteArray decodeKey(const QByteArray &rawKey);
    
    QByteArray key;  // Ключ шифрования
};

#endif

