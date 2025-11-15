#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QString>
#include <QList>

class QGridLayout;
class QWidget;
class QLabel;

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
    // Создание виджетов для отображения записей в сетке
    void createRecordWidgets();

    QWidget *centralWidget;
    QGridLayout *gridLayout;
    QList<InvoiceRecord> records;
};

#endif
