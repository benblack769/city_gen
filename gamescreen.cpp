#include <qgraphicsscene.h>
#include <QKeyEvent>
#include <ctime>
#include <iostream>

#include <QGraphicsSceneMouseEvent>
#include "gamescreen.h"

GameScreen::GameScreen()
{
    this->addItem(&pix);
    img_data.resize(sqr(WORLD_SIZE));
    frame_timer.setInterval(100);
    frame_timer.start();
    connect(&frame_timer,&QTimer::timeout,this,&GameScreen::draw);
}
void GameScreen::keyPressEvent(QKeyEvent * event){
    int k = event->key();
}
void GameScreen::mousePressEvent(QGraphicsSceneMouseEvent *mouseEvent){
    //this function currently does nothing at all
    QGraphicsScene::mousePressEvent(mouseEvent);
}
void GameScreen::mouseMoveEvent(QGraphicsSceneMouseEvent *mouseEvent){
    //this function currently does nothing at all
    QGraphicsScene::mouseMoveEvent(mouseEvent);
}
rgba color_convert(QColor color){
    int r,g,b,a;
    color.getRgb(&r,&g,&b,&a);
    return rgba{uchar(r),uchar(g),uchar(b),uchar(a)};
}

void GameScreen::draw_thing(blocks::count_ty & thing, QColor color,double max_opacity){

    size_t max_count = *std::max_element(thing.Arr.begin(),thing.Arr.end());
    double mul_value = (255.0*max_opacity)/max_count;
    
    rgba mycolor = color_convert(color);
    for(size_t i = 0; i < blocks::arrsize(); i++){
        rgba this_c = mycolor;
        size_t t_count = thing.Arr[i];
        this_c.a = uchar(t_count*mul_value);
        img_data[i] = this_c;
    }
    uchar * data = reinterpret_cast<uchar *>(img_data.data());
    QImage img(data,WORLD_SIZE,WORLD_SIZE,QImage::Format_RGBA8888);
    
    pix.setPixmap(QPixmap::fromImage(img));// = std::unique_ptr<QGraphicsPixmapItem>(this->addPixmap(QPixmap::fromImage(*img)));
}
void GameScreen::draw(){
    draw_thing(screen_data.num_residents,Qt::black,1.0);
}
