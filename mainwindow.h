#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QString>
#include <QList>

class QGridLayout;
class QWidget;
class QLabel;
class QPushButton;
class EncryptionManager;

// Структура записи товарной накладной
struct InvoiceRecord
{
    QString article;        // Артикул товара (10 цифр)
    int quantity;           // Количество единиц товара
    qint64 timestamp;      // Дата и время отгрузки (unix timestamp)
    QString hash;          // Хеш MD5 в кодировке base64
    bool valid;            // Признак валидности записи (для подсветки)
    
    InvoiceRecord()
        : quantity(0)
        , timestamp(0)
        , valid(true)
    {
    }
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    // Настройка интерфейса с QGridLayout для отображения данных
    void setupUI();
    // Загрузка данных из JSON файла при старте приложения
    void loadDataFromFile();
    // Получение пути к файлу с данными
    QString getDataFilePath();
    // Загрузка и расшифровка файла (если зашифрован)
    QByteArray loadAndDecryptFile(const QString &filePath, QString &errorMessage);
    // Проверка, является ли файл зашифрованным (по расширению .enc)
    bool isEncryptedFile(const QString &filePath) const;
    // Парсинг JSON данных из байтового массива
    bool parseJsonData(const QByteArray &data);
    // Парсинг JSON файла и заполнение списка записей (устаревший метод, используйте loadAndDecryptFile + parseJsonData)
    bool parseJsonFile(const QString &filePath);
    // Проверка цепочки хешей MD5 для всех записей
    void verifyHashChain();
    // Вычисление MD5 хеша записи по формуле: hash_i = MD5(article + quantity + timestamp + hash_i-1)
    QString computeHash(const InvoiceRecord &record, const QString &previousHash);
    // Отображение всех записей в сетке QGridLayout
    void displayRecords();
    // Обработчик нажатия кнопки "Открыть"
    void onOpenButtonClicked();

    QWidget *centralWidget;
    QWidget *gridWidget;
    QGridLayout *gridLayout;
    QPushButton *openButton;
    QList<InvoiceRecord> records;
    QString currentFilePath;
    EncryptionManager *encryptionManager;  // Менеджер шифрования для расшифровки файлов
};

#endif
