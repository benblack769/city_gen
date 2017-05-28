#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "gamescreen.h"
#include <qgraphicsview.h>
#include <QVBoxLayout>
#include <QLabel>
#include <QCheckBox>
#include <ctime>
#include <iostream>
using namespace std;

const int max_slide = 20;
const int max_time = 500;

inline int slide_time(int slide){
    //todo:change to proper logarithmic scale
    return (slide *  max_time) / max_slide;
}

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    
    screen = new GameScreen;
    screen->setSceneRect(QRectF(0, 0, WORLD_SIZE, WORLD_SIZE));
    veiw = new QGraphicsView(screen);
    
    veiw->setTransform(QTransform().scale(5,5));

    
    QHBoxLayout * lay = new QHBoxLayout(this);
    lay->addWidget(make_layout());
    lay->addWidget(veiw,Qt::AlignLeft);
    QWidget * W = new QWidget;
    W->setLayout(lay);
    setCentralWidget(W);

    setWindowTitle(tr("Economy Simulator"));
    
    frame_timer.setInterval(50);
    frame_timer.start();
    connect(&frame_timer,&QTimer::timeout,this,&MainWindow::draw);
    update_timer.setInterval(5);
    update_timer.start();
    connect(&update_timer,&QTimer::timeout,[&](){world.update();});
}
QWidget * MainWindow::make_layout(){
    QWidget * W = new QWidget(this);
    
    add_checkbox(QString("residents"),W,true,Qt::blue,[](PointProperty pp){return pp.residents.size();});
    add_checkbox(QString("food_level"),W,true,Qt::green,[](PointProperty pp){return pp.food_content;});
    
    QLabel * sclab = new QLabel(QString("Speed Control"));
    QSlider * scslide = new QSlider(this);
    connect(scslide,&QSlider::sliderMoved,[&](int slide_val){
        frame_timer.setInterval(slide_time(slide_val));
    });
    
    QLabel * zclab = new QLabel(QString("Zoom Control"));
    QSlider * zcslide = new QSlider(this);
    connect(zcslide,&QSlider::sliderMoved,[&](int slide_val){
        veiw->setTransform(QTransform().scale(slide_val,slide_val));
    });
    
    QVBoxLayout * lay = new QVBoxLayout(this);
    for(QCheckBox * ck : chks){
        lay->addWidget(ck,0,Qt::AlignTop);
    }
    lay->addWidget(zclab);
    lay->addWidget(zcslide,0,Qt::AlignTop);
    lay->addWidget(sclab);
    lay->addWidget(scslide,10,Qt::AlignTop);
    W->setLayout(lay);
    return W;
}
void MainWindow::draw(){
    screen->draw_map(world.map,colorers);
}
MainWindow::~MainWindow()
{
    delete ui;
    //delete others?
}
