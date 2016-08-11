#include "mainwindow.h"
#include <QApplication>
#include "constants.h"
#include "test.h"

int main(int argc, char *argv[])
{
#ifdef RUN_TESTS
    all_tests.collect_tests();
    all_tests.run_all();
#else
    QApplication a(argc, argv);
    MainWindow w;
    w.show();

    return a.exec();
#endif
}
