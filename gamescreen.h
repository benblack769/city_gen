#pragma once
#include <QGraphicsScene>
#include <blocks.h>
#include <QTimer>
#include <memory>
#include <QGraphicsPixmapItem>

class GameScreen : public QGraphicsScene
{
public:
    blocks screen_data;
    QTimer frame_timer;
    std::unique_ptr<QGraphicsPixmapItem> pix;

    GameScreen();
    void keyPressEvent(QKeyEvent * event);
    void mousePressEvent(QGraphicsSceneMouseEvent *mouseEvent);
    void mouseMoveEvent(QGraphicsSceneMouseEvent *mouseEvent);
    void draw();
    void draw_thing(blocks::count_ty & thing, QColor color,double max_opacity);
};
