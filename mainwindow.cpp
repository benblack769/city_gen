#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "gamescreen.h"
#include <qgraphicsview.h>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    screen = new GameScreen;
    screen->setSceneRect(QRectF(0, 0, WORLD_SIZE, WORLD_SIZE));
    veiw = new QGraphicsView(screen);
    veiw->setTransform(QTransform().scale(10,10));

    setCentralWidget(veiw);


    setWindowTitle(tr("Economy Simulator"));
}

MainWindow::~MainWindow()
{
    delete ui;
}
