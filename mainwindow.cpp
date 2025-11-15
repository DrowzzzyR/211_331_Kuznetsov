#include "mainwindow.h"
#include "encryptionmanager.h"
#include <QGridLayout>
#include <QLabel>
#include <QWidget>
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
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
#include <QFileDialog>
#include <QCryptographicHash>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , encryptionManager(nullptr)
{
    setWindowTitle("211_331_Kuznetsov — Товарные накладные");
    setMinimumSize(800, 600);
    
    // Инициализируем менеджер шифрования
    encryptionManager = new EncryptionManager();
    
    // Пытаемся загрузить ключ шифрования из файла
    // Ключ должен находиться в config/encryption.key относительно приложения
    QString keyPath = QCoreApplication::applicationDirPath() + "/config/encryption.key";
    QString error;
    if (!encryptionManager->loadKeyFromFile(keyPath, error)) {
        // Если ключ не найден, пробуем найти в других местах
        QDir appDir(QCoreApplication::applicationDirPath());
        for (int i = 0; i < 5; ++i) {
            QString candidate = appDir.filePath("config/encryption.key");
            if (QFile::exists(candidate)) {
                if (encryptionManager->loadKeyFromFile(candidate, error)) {
                    qDebug() << "MainWindow::MainWindow: Ключ шифрования загружен из:" << candidate;
                    break;
                }
            }
            if (!appDir.cdUp()) {
                break;
            }
        }
        
        if (!encryptionManager->isReady()) {
            qDebug() << "MainWindow::MainWindow: Предупреждение - ключ шифрования не загружен. Зашифрованные файлы не будут поддерживаться.";
            qDebug() << "MainWindow::MainWindow: Ошибка:" << error;
        }
    } else {
        qDebug() << "MainWindow::MainWindow: Ключ шифрования успешно загружен из:" << keyPath;
    }
    
    setupUI();
    loadDataFromFile();
}

MainWindow::~MainWindow()
{
    if (encryptionManager) {
        delete encryptionManager;
        encryptionManager = nullptr;
    }
}

void MainWindow::setupUI()
{
    // Создаём центральный виджет и основной вертикальный layout
    centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);
    
    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->setContentsMargins(10, 10, 10, 10);
    mainLayout->setSpacing(10);
    
    // Создаём горизонтальный layout для кнопки "Открыть"
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    openButton = new QPushButton("Открыть", centralWidget);
    openButton->setMinimumWidth(100);
    connect(openButton, &QPushButton::clicked, this, &MainWindow::onOpenButtonClicked);
    buttonLayout->addWidget(openButton);
    buttonLayout->addStretch(); // Растягиваем пространство справа
    mainLayout->addLayout(buttonLayout);
    
    // Создаём QGridLayout для табличного отображения записей
    gridWidget = new QWidget(centralWidget);
    gridLayout = new QGridLayout(gridWidget);
    gridLayout->setSpacing(5);
    gridLayout->setContentsMargins(0, 0, 0, 0);
    
    // Добавляем заголовки колонок в первую строку сетки
    QLabel *articleHeader = new QLabel("Артикул", gridWidget);
    QLabel *quantityHeader = new QLabel("Количество", gridWidget);
    QLabel *dateHeader = new QLabel("Дата отгрузки", gridWidget);
    QLabel *hashHeader = new QLabel("Хеш", gridWidget);
    
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
    
    // Добавляем виджет с сеткой в основной layout
    mainLayout->addWidget(gridWidget, 1); // Растягиваем по вертикали
    
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

bool MainWindow::isEncryptedFile(const QString &filePath) const
{
    QFileInfo fileInfo(filePath);
    QString suffix = fileInfo.suffix().toLower();
    return suffix == "enc";
}

QByteArray MainWindow::loadAndDecryptFile(const QString &filePath, QString &errorMessage)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        errorMessage = QString("Не удалось открыть файл: %1").arg(file.errorString());
        return QByteArray();
    }
    
    QByteArray data = file.readAll();
    file.close();
    
    // Проверяем, зашифрован ли файл
    if (isEncryptedFile(filePath)) {
        qDebug() << "MainWindow::loadAndDecryptFile: Обнаружен зашифрованный файл:" << filePath;
        
        // Проверяем, готов ли менеджер шифрования
        if (!encryptionManager || !encryptionManager->isReady()) {
            errorMessage = "Ключ шифрования не загружен. Невозможно расшифровать файл.";
            return QByteArray();
        }
        
        // Расшифровываем данные
        QString decryptError;
        QByteArray decryptedData = encryptionManager->decrypt(data, decryptError);
        
        if (decryptedData.isEmpty()) {
            errorMessage = QString("Ошибка расшифровки: %1").arg(decryptError);
            return QByteArray();
        }
        
        qDebug() << "MainWindow::loadAndDecryptFile: Файл успешно расшифрован, размер данных:" << decryptedData.size();
        return decryptedData;
    } else {
        // Файл не зашифрован, возвращаем как есть
        qDebug() << "MainWindow::loadAndDecryptFile: Файл не зашифрован, загружаем как обычный JSON";
        return data;
    }
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
    
    // Загружаем и расшифровываем файл (если зашифрован)
    QString errorMessage;
    QByteArray data = loadAndDecryptFile(filePath, errorMessage);
    
    if (data.isEmpty()) {
        QMessageBox::warning(this, "Ошибка загрузки",
                            "Не удалось загрузить данные из файла.\n\n"
                            "Файл: " + filePath + "\n\n"
                            "Ошибка: " + errorMessage);
        return;
    }
    
    // Парсим JSON данные
    if (!parseJsonData(data)) {
        QMessageBox::warning(this, "Ошибка парсинга",
                            "Не удалось распарсить данные из файла.\n\n"
                            "Файл: " + filePath + "\n\n"
                            "Проверьте, что файл имеет правильный формат JSON.");
        return;
    }
    
    qDebug() << "MainWindow::loadDataFromFile: Успешно загружено записей:" << records.size();
    
    // Проверяем цепочку хешей после загрузки
    verifyHashChain();
    
    displayRecords();
    
    // Сохраняем путь к текущему файлу
    currentFilePath = filePath;
}

bool MainWindow::parseJsonData(const QByteArray &data)
{
    QJsonParseError parseError;
    QJsonDocument document = QJsonDocument::fromJson(data, &parseError);
    
    if (parseError.error != QJsonParseError::NoError) {
        qDebug() << "MainWindow::parseJsonData: Ошибка парсинга JSON:" << parseError.errorString()
                 << "позиция:" << parseError.offset;
        return false;
    }
    
    if (!document.isArray()) {
        qDebug() << "MainWindow::parseJsonData: Документ не является массивом";
        return false;
    }
    
    QJsonArray array = document.array();
    records.clear();
    records.reserve(array.size());
    
    for (int i = 0; i < array.size(); ++i) {
        QJsonValue value = array.at(i);
        if (!value.isObject()) {
            qDebug() << "MainWindow::parseJsonData: Элемент #" << i << "не является объектом";
            continue;
        }
        
        QJsonObject obj = value.toObject();
        InvoiceRecord record;
        
        // Парсинг артикула (10 цифр)
        record.article = obj.value("article").toString();
        if (record.article.length() != 10 || !record.article.toLongLong()) {
            qDebug() << "MainWindow::parseJsonData: Некорректный артикул в записи #" << i;
            continue;
        }
        
        // Парсинг количества
        record.quantity = obj.value("quantity").toInt();
        if (record.quantity <= 0) {
            qDebug() << "MainWindow::parseJsonData: Некорректное количество в записи #" << i;
            continue;
        }
        
        // Парсинг timestamp
        record.timestamp = obj.value("timestamp").toVariant().toLongLong();
        if (record.timestamp <= 0) {
            qDebug() << "MainWindow::parseJsonData: Некорректный timestamp в записи #" << i;
            continue;
        }
        
        // Парсинг хеша
        record.hash = obj.value("hash").toString();
        if (record.hash.isEmpty()) {
            qDebug() << "MainWindow::parseJsonData: Отсутствует хеш в записи #" << i;
            continue;
        }
        
        record.valid = true; // Пока что все записи считаем валидными
        records.append(record);
    }
    
    qDebug() << "MainWindow::parseJsonData: Успешно распарсено записей:" << records.size();
    return !records.isEmpty();
}

bool MainWindow::parseJsonFile(const QString &filePath)
{
    // Устаревший метод, используем новый подход с расшифровкой
    QString errorMessage;
    QByteArray data = loadAndDecryptFile(filePath, errorMessage);
    
    if (data.isEmpty()) {
        qDebug() << "MainWindow::parseJsonFile: Не удалось загрузить файл:" << errorMessage;
        return false;
    }
    
    return parseJsonData(data);
}

QString MainWindow::computeHash(const InvoiceRecord &record, const QString &previousHash)
{
    // Формула: hash_i = MD5(article + quantity + timestamp + hash_i-1)
    // Все поля преобразуем в строку и объединяем
    QString data = record.article + 
                   QString::number(record.quantity) + 
                   QString::number(record.timestamp) + 
                   previousHash;
    
    // Вычисляем MD5 хеш
    QCryptographicHash hash(QCryptographicHash::Md5);
    hash.addData(data.toUtf8());
    QByteArray hashResult = hash.result();
    
    // Возвращаем в формате base64
    return hashResult.toBase64();
}

void MainWindow::verifyHashChain()
{
    QString previousHash; // Для первой записи previousHash пустой
    
    for (int i = 0; i < records.size(); ++i) {
        InvoiceRecord &record = records[i];
        
        // Вычисляем ожидаемый хеш
        QString expectedHash = computeHash(record, previousHash);
        
        // Сравниваем с хешем из файла
        if (record.hash == expectedHash) {
            record.valid = true;
            previousHash = record.hash; // Используем хеш из файла для следующей записи
        } else {
            // Если хеш не совпадает, помечаем эту и все последующие записи как невалидные
            record.valid = false;
            qDebug() << "MainWindow::verifyHashChain: Обнаружено нарушение целостности в записи #" << (i + 1)
                     << "Ожидаемый хеш:" << expectedHash
                     << "Хеш из файла:" << record.hash;
            
            // Помечаем все последующие записи как невалидные
            for (int j = i + 1; j < records.size(); ++j) {
                records[j].valid = false;
            }
            break;
        }
    }
    
    qDebug() << "MainWindow::verifyHashChain: Проверка целостности завершена";
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
        QLabel *articleLabel = new QLabel(record.article, gridWidget);
        
        // Количество
        QLabel *quantityLabel = new QLabel(QString::number(record.quantity), gridWidget);
        
        // Дата отгрузки (преобразуем unix timestamp в читаемый формат)
        QDateTime dateTime = QDateTime::fromSecsSinceEpoch(record.timestamp);
        QLabel *dateLabel = new QLabel(dateTime.toString("dd.MM.yyyy hh:mm:ss"), gridWidget);
        
        // Хеш
        QLabel *hashLabel = new QLabel(record.hash, gridWidget);
        hashLabel->setWordWrap(true); // Перенос длинного хеша
        
        // Подсветка невалидных записей красным цветом с белым текстом
        if (!record.valid) {
            QString invalidStyle = "background-color: #dc3545; color: white; padding: 5px;";
            articleLabel->setStyleSheet(invalidStyle);
            quantityLabel->setStyleSheet(invalidStyle);
            dateLabel->setStyleSheet(invalidStyle);
            hashLabel->setStyleSheet(invalidStyle);
        }
        
        // Добавляем виджеты в сетку
        gridLayout->addWidget(articleLabel, row, 0);
        gridLayout->addWidget(quantityLabel, row, 1);
        gridLayout->addWidget(dateLabel, row, 2);
        gridLayout->addWidget(hashLabel, row, 3);
    }
    
    qDebug() << "MainWindow::displayRecords: Отображено записей в сетке:" << records.size();
}

void MainWindow::onOpenButtonClicked()
{
    // Открываем диалог выбора файла
    QString initialDir;
    if (currentFilePath.isEmpty()) {
        QString defaultPath = getDataFilePath();
        QFileInfo fileInfo(defaultPath);
        initialDir = fileInfo.absolutePath();
    } else {
        QFileInfo fileInfo(currentFilePath);
        initialDir = fileInfo.absolutePath();
    }
    
    // Убеждаемся, что начальная директория существует
    QDir dir(initialDir);
    if (!dir.exists()) {
        initialDir = QDir::homePath();
    }
    
    qDebug() << "MainWindow::onOpenButtonClicked: Начальная директория:" << initialDir;
    
    // Обновляем фильтр для поддержки зашифрованных файлов
    // Включаем оба типа файлов в первый фильтр, чтобы они были видны по умолчанию
    QString selectedFile = QFileDialog::getOpenFileName(
        this,
        "Выбор файла с данными",
        initialDir,
        "Все поддерживаемые файлы (*.json *.enc);;JSON файлы (*.json);;Зашифрованные файлы (*.enc);;Все файлы (*.*)",
        nullptr,
        QFileDialog::DontResolveSymlinks
    );
    
    if (selectedFile.isEmpty()) {
        return; // Пользователь отменил выбор
    }
    
    qDebug() << "MainWindow::onOpenButtonClicked: Выбран файл:" << selectedFile;
    
    // Загружаем и расшифровываем файл (если зашифрован)
    QString errorMessage;
    QByteArray data = loadAndDecryptFile(selectedFile, errorMessage);
    
    if (data.isEmpty()) {
        QMessageBox::warning(this, "Ошибка загрузки",
                            "Не удалось загрузить данные из файла.\n\n"
                            "Файл: " + selectedFile + "\n\n"
                            "Ошибка: " + errorMessage);
        return;
    }
    
    // Парсим JSON данные
    if (!parseJsonData(data)) {
        QMessageBox::warning(this, "Ошибка парсинга",
                            "Не удалось распарсить данные из файла.\n\n"
                            "Файл: " + selectedFile + "\n\n"
                            "Проверьте, что файл имеет правильный формат JSON.");
        return;
    }
    
    qDebug() << "MainWindow::onOpenButtonClicked: Успешно загружено записей:" << records.size();
    
    // Проверяем цепочку хешей
    verifyHashChain();
    
    // Отображаем записи
    displayRecords();
    
    // Сохраняем путь к текущему файлу
    currentFilePath = selectedFile;
}
