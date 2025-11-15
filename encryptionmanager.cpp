#include "encryptionmanager.h"
#include <QFile>
#include <QFileInfo>
#include <QDebug>
#include <openssl/evp.h>
#include <openssl/rand.h>

EncryptionManager::EncryptionManager()
{
}

bool EncryptionManager::setKey(const QByteArray &key)
{
    if (key.size() != KEY_SIZE) {
        return false;
    }
    this->key = key;
    return true;
}

bool EncryptionManager::isReady() const
{
    return key.size() == KEY_SIZE;
}

QByteArray EncryptionManager::decodeKey(const QByteArray &rawKey)
{
    QByteArray trimmed = rawKey.trimmed();
    if (trimmed.isEmpty()) {
        return QByteArray();
    }
    
    // Пробуем base64
    QByteArray decoded = QByteArray::fromBase64(trimmed, QByteArray::Base64Option::Base64Encoding);
    if (decoded.size() == KEY_SIZE) {
        return decoded;
    }
    
    // Пробуем hex
    decoded = QByteArray::fromHex(trimmed);
    if (decoded.size() == KEY_SIZE) {
        return decoded;
    }
    
    // Если размер уже 32 байта, используем как есть
    if (trimmed.size() == KEY_SIZE) {
        return trimmed;
    }
    
    return QByteArray();
}

bool EncryptionManager::loadKeyFromFile(const QString &filePath, QString &errorMessage)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        errorMessage = QString("Не удалось открыть файл ключа: %1").arg(file.errorString());
        return false;
    }
    
    QByteArray content = file.readAll();
    file.close();
    
    QByteArray parsedKey = decodeKey(content);
    if (parsedKey.size() != KEY_SIZE) {
        errorMessage = QString("Некорректный формат ключа. Ожидается 32 байта (base64 или hex).");
        return false;
    }
    
    key = parsedKey;
    qDebug() << "EncryptionManager::loadKeyFromFile: Ключ успешно загружен из файла:" << filePath;
    return true;
}

QByteArray EncryptionManager::encrypt(const QByteArray &plainData, QString &errorMessage) const
{
    if (!isReady()) {
        errorMessage = QString("Ключ шифрования не загружен.");
        return QByteArray();
    }
    
    // Генерируем случайный IV
    unsigned char iv[IV_SIZE];
    if (RAND_bytes(iv, IV_SIZE) != 1) {
        errorMessage = QString("Не удалось сгенерировать IV.");
        return QByteArray();
    }
    
    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    if (!ctx) {
        errorMessage = QString("Не удалось инициализировать контекст шифрования.");
        return QByteArray();
    }
    
    QByteArray ciphertext;
    ciphertext.resize(plainData.size() + EVP_CIPHER_block_size(EVP_aes_256_cbc()));
    int outLen1 = 0;
    int outLen2 = 0;
    
    bool success = EVP_EncryptInit_ex(ctx,
                                      EVP_aes_256_cbc(),
                                      nullptr,
                                      reinterpret_cast<const unsigned char*>(key.constData()),
                                      iv) == 1;
    if (success) {
        success = EVP_EncryptUpdate(ctx,
                                   reinterpret_cast<unsigned char*>(ciphertext.data()),
                                   &outLen1,
                                   reinterpret_cast<const unsigned char*>(plainData.constData()),
                                   plainData.size()) == 1;
    }
    if (success) {
        success = EVP_EncryptFinal_ex(ctx,
                                     reinterpret_cast<unsigned char*>(ciphertext.data()) + outLen1,
                                     &outLen2) == 1;
    }
    
    EVP_CIPHER_CTX_free(ctx);
    
    if (!success) {
        errorMessage = QString("Ошибка при шифровании данных.");
        return QByteArray();
    }
    
    ciphertext.resize(outLen1 + outLen2);
    
    // Формируем результат: IV + зашифрованные данные
    QByteArray result;
    result.reserve(IV_SIZE + ciphertext.size());
    result.append(reinterpret_cast<const char*>(iv), IV_SIZE);
    result.append(ciphertext);
    
    return result;
}

QByteArray EncryptionManager::decrypt(const QByteArray &encryptedData, QString &errorMessage) const
{
    if (!isReady()) {
        errorMessage = QString("Ключ шифрования не загружен.");
        return QByteArray();
    }
    
    if (encryptedData.size() <= IV_SIZE) {
        errorMessage = QString("Шифротекст повреждён: недостаточно данных.");
        return QByteArray();
    }
    
    // Извлекаем IV и зашифрованные данные
    const unsigned char *iv = reinterpret_cast<const unsigned char*>(encryptedData.constData());
    const unsigned char *ciphertext = iv + IV_SIZE;
    int ciphertext_len = encryptedData.size() - IV_SIZE;
    
    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    if (!ctx) {
        errorMessage = QString("Не удалось инициализировать контекст расшифровки.");
        return QByteArray();
    }
    
    QByteArray plaintext;
    plaintext.resize(ciphertext_len + EVP_CIPHER_block_size(EVP_aes_256_cbc()));
    int outLen1 = 0;
    int outLen2 = 0;
    
    bool success = EVP_DecryptInit_ex(ctx,
                                      EVP_aes_256_cbc(),
                                      nullptr,
                                      reinterpret_cast<const unsigned char*>(key.constData()),
                                      iv) == 1;
    if (success) {
        success = EVP_DecryptUpdate(ctx,
                                   reinterpret_cast<unsigned char*>(plaintext.data()),
                                   &outLen1,
                                   ciphertext,
                                   ciphertext_len) == 1;
    }
    if (success) {
        success = EVP_DecryptFinal_ex(ctx,
                                     reinterpret_cast<unsigned char*>(plaintext.data()) + outLen1,
                                     &outLen2) == 1;
    }
    
    EVP_CIPHER_CTX_free(ctx);
    
    if (!success) {
        errorMessage = QString("Ошибка при расшифровке данных. Возможно, неверный ключ.");
        return QByteArray();
    }
    
    plaintext.resize(outLen1 + outLen2);
    return plaintext;
}

