#include <qgraphicsscene.h>
#include <QKeyEvent>

#include <QGraphicsSceneMouseEvent>
#include "gamescreen.h"

GameScreen::GameScreen()
{
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
void GameScreen::draw_thing(blocks::count_ty & thing, QColor color,double max_opacity){
    QImage * img = new QImage(WORLD_SIZE,WORLD_SIZE,QImage::Format_RGBA8888);

    size_t max_count = *std::max_element(thing.Arr.begin(),thing.Arr.end());

    for(Point p : iter_all()){
        QColor pix_color = color;
        size_t t_count = thing[p];
        pix_color.setAlphaF((max_opacity*t_count)/max_count);
        img->setPixel(p.X,p.Y,pix_color.rgba());
    }
    pix = std::unique_ptr<QGraphicsPixmapItem>(this->addPixmap(QPixmap::fromImage(*img)));
}
void GameScreen::draw(){
    draw_thing(screen_data.num_residents,Qt::black,1.0);
}
