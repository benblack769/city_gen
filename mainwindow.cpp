#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "gamescreen.h"
#include <qgraphicsview.h>
#include <QVBoxLayout>
#include <ctime>
#include <iostream>
using namespace std;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    
    screen = new GameScreen;
    screen->setSceneRect(QRectF(0, 0, WORLD_SIZE, WORLD_SIZE));
    veiw = new QGraphicsView(screen);
    veiw->setTransform(QTransform().scale(10,10));

    
    QHBoxLayout * lay = new QHBoxLayout(this);
    lay->addWidget(make_layout());
    lay->addWidget(veiw,Qt::AlignLeft);
    QWidget * W = new QWidget;
    W->setLayout(lay);
    setCentralWidget(W);

    setWindowTitle(tr("Economy Simulator"));
    
    
    frame_timer.setInterval(250);
    frame_timer.start();
    connect(&frame_timer,&QTimer::timeout,this,&MainWindow::draw);
}

QWidget * MainWindow::make_layout(){
    QWidget * W = new QWidget(this);
    QVBoxLayout * lay = new QVBoxLayout(this);
    chks[0] = check_obj("num_residents",false,[&](){screen->draw_thing(screen->screen_data.num_residents,Qt::black, 0.5);},W);
    chks[1] = check_obj("num_workers",false,[&](){screen->draw_thing(screen->screen_data.num_workers,Qt::blue, 0.5);},W);
    chks[2] = check_obj("trans_usage",false,[&](){screen->draw_thing(screen->screen_data.trans_usage,Qt::red, 0.5);},W);
    chks[3] = check_obj("trans_invest",true,[&](){screen->draw_thing(screen->screen_data.trans_invest,Qt::yellow, 0.5);},W);
    chks[4] = check_obj("size_t_upgrade_vs",true,[&](){screen->draw_thing(screen->screen_data.size_t_upgrade_vs,Qt::green, 0.5);},W);
    for(check_obj & ck : chks){
        lay->addWidget(ck.box,100,Qt::AlignTop);
    }
    W->setLayout(lay);
    return W;
}
void MainWindow::draw(){
    clock_t st = clock();
    screen->clear();
    clock_t algost = clock();
    screen->screen_data.update();
    clock_t algotot = clock() - algost;
    for(check_obj & ck : chks){
        ck.call_if_checked();
    }
    clock_t tot = clock() - st;
    cout << "Algorithm time update: " << algotot << endl;
    cout << "Total time update: " << tot << endl;
}


MainWindow::~MainWindow()
{
    delete ui;
}
