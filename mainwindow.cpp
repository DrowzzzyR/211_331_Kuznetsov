#include "mainwindow.h"
#include <QGridLayout>
#include <QLabel>
#include <QWidget>
#include <QDebug>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle("211_331_Kuznetsov — Товарные накладные");
    setMinimumSize(800, 600);
    
    setupUI();
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
    
    createRecordWidgets();
}

// Метод для будущего добавления записей в сетку
// Будет вызываться после загрузки данных из JSON файла

void MainWindow::createRecordWidgets()
{
    // Пока что только заголовки, записи будут добавлены при загрузке данных из JSON файла
    // Это базовая структура для отображения данных в сетке QGridLayout
    qDebug() << "MainWindow::createRecordWidgets: Заголовки таблицы созданы, ожидается загрузка данных";
}
