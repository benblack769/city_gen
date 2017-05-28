#pragma once
#include <QGraphicsScene>
#include <memory>
#include <functional>
#include <QGraphicsPixmapItem>

#include <map.h>

struct rgba{
    uchar r;
    uchar g;
    uchar b;
    uchar a;
};

rgba color_convert(QColor color){
    int r,g,b,a;
    color.getRgb(&r,&g,&b,&a);
    return rgba{uchar(r),uchar(g),uchar(b),uchar(a)};
}
struct ScreenSettings{
    bool filler;
};

class ScreenColorer{
public:
    ScreenSettings mysettings;
    ScreenColorer(Map & m,ScreenSettings settings):
        mysettings(settings){}
    virtual QColor point_col(PointProperty prop){
        return prop.residents.size() ? Qt::blue : Qt::white;
    }
};

class GameScreen : public QGraphicsScene
{
public:
    GameScreen(){};
    void keyPressEvent(QKeyEvent * event){
        //this function currently does nothing at all
        //int k = event->key();
        QGraphicsScene::keyPressEvent(event);
    }
    void mousePressEvent(QGraphicsSceneMouseEvent *mouseEvent){
        //this function currently does nothing at all
        QGraphicsScene::mousePressEvent(mouseEvent);
    }
    void draw_map(Map & map, ScreenSettings settings){
        this->clear();
        ScreenColorer colorer(map,settings);
        
        board<rgba> color_data;
        for(Point P : iter_all()){
            QColor col = colorer.point_col(map[P]);
            color_data[P] = color_convert(col);
        }
        
        uchar * data = reinterpret_cast<uchar *>(color_data.Arr.data());
        QImage img(data,WORLD_SIZE,WORLD_SIZE,QImage::Format_RGBA8888);
        
        this->addPixmap(QPixmap::fromImage(img));// = std::unique_ptr<QGraphicsPixmapItem>(this->addPixmap(QPixmap::fromImage(*img)));
    }
};
