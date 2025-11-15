#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QString>
#include <QList>

class QGridLayout;
class QWidget;
class QLabel;
class QPushButton;

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
    // Парсинг JSON файла и заполнение списка записей
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
};

#endif
