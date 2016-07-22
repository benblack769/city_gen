#pragma once
#include <QGraphicsScene>
#include <blocks.h>
#include <QTimer>
#include <memory>
#include <QGraphicsPixmapItem>

struct rgba{
    uchar r;
    uchar g;
    uchar b;
    uchar a;
};

class GameScreen : public QGraphicsScene
{
public:
    blocks screen_data;
    QTimer frame_timer;
    QGraphicsPixmapItem pix;
    vector<rgba> img_data;
    

    GameScreen();
    void keyPressEvent(QKeyEvent * event);
    void mousePressEvent(QGraphicsSceneMouseEvent *mouseEvent);
    void mouseMoveEvent(QGraphicsSceneMouseEvent *mouseEvent);
    void draw();
    void draw_thing(blocks::count_ty & thing, QColor color,double max_opacity);
};
