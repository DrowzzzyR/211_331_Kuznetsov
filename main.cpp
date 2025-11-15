#include "mainwindow.h"
#include "integritycheck.h"
#include <QApplication>
#include <QMessageBox>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

#ifndef _DEBUG
    if (!IntegrityCheck::verifyTextSegment()) {
        QMessageBox::critical(nullptr, "Обнаружена атака",
            "Обнаружена модификация исполняемого файла!\n\n"
            "Контрольная сумма сегмента .text не совпадает с эталонной.\n"
            "Приложение могло быть изменено злоумышленником.\n\n"
            "Приложение будет закрыто для защиты данных.");
        return 1;
    }
#endif

    MainWindow w;
    w.show();

    return a.exec();
}
