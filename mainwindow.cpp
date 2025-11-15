#include "mainwindow.h"
#include <QGridLayout>
#include <QLabel>
#include <QWidget>
#include <QDebug>
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QCoreApplication>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QDateTime>
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle("211_331_Kuznetsov — Товарные накладные");
    setMinimumSize(800, 600);
    
    setupUI();
    loadDataFromFile();
}

MainWindow::~MainWindow()
{
}

void MainWindow::setupUI()
{
    // Создаём центральный виджет и сетку для отображения данных
    centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);
    
    // Создаём QGridLayout для табличного отображения записей
    gridLayout = new QGridLayout(centralWidget);
    gridLayout->setSpacing(5);
    gridLayout->setContentsMargins(10, 10, 10, 10);
    
    // Добавляем заголовки колонок в первую строку сетки
    QLabel *articleHeader = new QLabel("Артикул", centralWidget);
    QLabel *quantityHeader = new QLabel("Количество", centralWidget);
    QLabel *dateHeader = new QLabel("Дата отгрузки", centralWidget);
    QLabel *hashHeader = new QLabel("Хеш", centralWidget);
    
    // Стилизация заголовков
    articleHeader->setStyleSheet("font-weight: bold; font-size: 12pt; padding: 5px;");
    quantityHeader->setStyleSheet("font-weight: bold; font-size: 12pt; padding: 5px;");
    dateHeader->setStyleSheet("font-weight: bold; font-size: 12pt; padding: 5px;");
    hashHeader->setStyleSheet("font-weight: bold; font-size: 12pt; padding: 5px;");
    
    // Размещаем заголовки в первой строке сетки
    gridLayout->addWidget(articleHeader, 0, 0);
    gridLayout->addWidget(quantityHeader, 0, 1);
    gridLayout->addWidget(dateHeader, 0, 2);
    gridLayout->addWidget(hashHeader, 0, 3);
    
    // Настройка растягивания колонок для адаптивной раскладки
    // Это обеспечивает совместное изменение размеров элементов при изменении размеров окна
    gridLayout->setColumnStretch(0, 1);  // Артикул
    gridLayout->setColumnStretch(1, 1);  // Количество
    gridLayout->setColumnStretch(2, 2);  // Дата отгрузки
    gridLayout->setColumnStretch(3, 3);  // Хеш (самая широкая колонка)
    
    qDebug() << "MainWindow::setupUI: Интерфейс с QGridLayout успешно настроен";
}

QString MainWindow::getDataFilePath()
{
    // Ищем файл с данными относительно директории приложения
    QString appDir = QCoreApplication::applicationDirPath();
    QString relativePath = "data/invoices_valid.json";
    
    // Проверяем в директории приложения
    QDir dir(appDir);
    for (int i = 0; i < 5; ++i) {
        QString candidate = dir.filePath(relativePath);
        if (QFile::exists(candidate)) {
            return QFileInfo(candidate).canonicalFilePath();
        }
        if (!dir.cdUp()) {
            break;
        }
    }
    
    // Если не нашли, возвращаем путь относительно текущей директории
    QString fallback = QDir::current().filePath(relativePath);
    if (QFile::exists(fallback)) {
        return QFileInfo(fallback).canonicalFilePath();
    }
    
    return fallback;
}

void MainWindow::loadDataFromFile()
{
    QString filePath = getDataFilePath();
    qDebug() << "MainWindow::loadDataFromFile: Попытка загрузить данные из файла:" << filePath;
    qDebug() << "MainWindow::loadDataFromFile: Файл существует:" << QFile::exists(filePath);
    
    if (!QFile::exists(filePath)) {
        QMessageBox::warning(this, "Файл не найден",
                            "Не удалось найти файл с данными.\n\n"
                            "Ожидаемый путь: " + filePath + "\n\n"
                            "Убедитесь, что файл data/invoices_valid.json существует.");
        return;
    }
    
    if (!parseJsonFile(filePath)) {
        QMessageBox::warning(this, "Ошибка загрузки",
                            "Не удалось загрузить данные из файла.\n\n"
                            "Файл: " + filePath + "\n\n"
                            "Проверьте, что файл существует и имеет правильный формат JSON.");
        return;
    }
    
    qDebug() << "MainWindow::loadDataFromFile: Успешно загружено записей:" << records.size();
    displayRecords();
}

bool MainWindow::parseJsonFile(const QString &filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        qDebug() << "MainWindow::parseJsonFile: Не удалось открыть файл:" << file.errorString();
        return false;
    }
    
    QByteArray data = file.readAll();
    file.close();
    
    QJsonParseError parseError;
    QJsonDocument document = QJsonDocument::fromJson(data, &parseError);
    
    if (parseError.error != QJsonParseError::NoError) {
        qDebug() << "MainWindow::parseJsonFile: Ошибка парсинга JSON:" << parseError.errorString()
                 << "позиция:" << parseError.offset;
        return false;
    }
    
    if (!document.isArray()) {
        qDebug() << "MainWindow::parseJsonFile: Документ не является массивом";
        return false;
    }
    
    QJsonArray array = document.array();
    records.clear();
    records.reserve(array.size());
    
    for (int i = 0; i < array.size(); ++i) {
        QJsonValue value = array.at(i);
        if (!value.isObject()) {
            qDebug() << "MainWindow::parseJsonFile: Элемент #" << i << "не является объектом";
            continue;
        }
        
        QJsonObject obj = value.toObject();
        InvoiceRecord record;
        
        // Парсинг артикула (10 цифр)
        record.article = obj.value("article").toString();
        if (record.article.length() != 10 || !record.article.toLongLong()) {
            qDebug() << "MainWindow::parseJsonFile: Некорректный артикул в записи #" << i;
            continue;
        }
        
        // Парсинг количества
        record.quantity = obj.value("quantity").toInt();
        if (record.quantity <= 0) {
            qDebug() << "MainWindow::parseJsonFile: Некорректное количество в записи #" << i;
            continue;
        }
        
        // Парсинг timestamp
        record.timestamp = obj.value("timestamp").toVariant().toLongLong();
        if (record.timestamp <= 0) {
            qDebug() << "MainWindow::parseJsonFile: Некорректный timestamp в записи #" << i;
            continue;
        }
        
        // Парсинг хеша
        record.hash = obj.value("hash").toString();
        if (record.hash.isEmpty()) {
            qDebug() << "MainWindow::parseJsonFile: Отсутствует хеш в записи #" << i;
            continue;
        }
        
        record.valid = true; // Пока что все записи считаем валидными
        records.append(record);
    }
    
    qDebug() << "MainWindow::parseJsonFile: Успешно распарсено записей:" << records.size();
    return !records.isEmpty();
}

void MainWindow::displayRecords()
{
    // Очищаем сетку от предыдущих записей (кроме заголовков в строке 0)
    // Удаляем все виджеты начиная со строки 1 (индекс 4 и далее, так как 4 заголовка)
    int headerCount = 4; // Количество заголовков
    int currentCount = gridLayout->count();
    
    // Удаляем все элементы кроме заголовков (первые 4 элемента)
    for (int i = currentCount - 1; i >= headerCount; --i) {
        QLayoutItem *item = gridLayout->takeAt(i);
        if (item) {
            if (QWidget *widget = item->widget()) {
                widget->deleteLater();
            }
            delete item;
        }
    }
    
    // Добавляем записи в сетку, начиная со строки 1 (строка 0 - заголовки)
    for (int i = 0; i < records.size(); ++i) {
        const InvoiceRecord &record = records[i];
        int row = i + 1; // Строка в сетке (0 - заголовки)
        
        // Артикул
        QLabel *articleLabel = new QLabel(record.article, centralWidget);
        gridLayout->addWidget(articleLabel, row, 0);
        
        // Количество
        QLabel *quantityLabel = new QLabel(QString::number(record.quantity), centralWidget);
        gridLayout->addWidget(quantityLabel, row, 1);
        
        // Дата отгрузки (преобразуем unix timestamp в читаемый формат)
        QDateTime dateTime = QDateTime::fromSecsSinceEpoch(record.timestamp);
        QLabel *dateLabel = new QLabel(dateTime.toString("dd.MM.yyyy hh:mm:ss"), centralWidget);
        gridLayout->addWidget(dateLabel, row, 2);
        
        // Хеш
        QLabel *hashLabel = new QLabel(record.hash, centralWidget);
        hashLabel->setWordWrap(true); // Перенос длинного хеша
        gridLayout->addWidget(hashLabel, row, 3);
    }
    
    qDebug() << "MainWindow::displayRecords: Отображено записей в сетке:" << records.size();
}
