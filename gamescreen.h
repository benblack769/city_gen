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

inline rgba color_convert(QColor color){
    int r,g,b,a;
    color.getRgb(&r,&g,&b,&a);
    return rgba{uchar(r),uchar(g),uchar(b),uchar(a)};
}
struct ScreenSettings{
    bool filler;
};

inline double normalize_value(double val, double minval, double maxval){
    return (val-minval)/(maxval-minval+1e-10);
}

class ValueColorer{
public:
    bool is_active;
    QColor col;
    function<double(PointProperty)> pvalfn;
    ValueColorer(bool starts_active,QColor valuecolor,function<double(PointProperty)> point_val_fn){
        is_active = starts_active;
        col = valuecolor;
        pvalfn = point_val_fn;
    }
    board<rgba> color_data(Map & m){
        vector<double> all_vals;
        for(Point p : iter_all()){
            all_vals.push_back(pvalfn(m[p]));
        }
        double maxval = *max_element(all_vals.begin(),all_vals.end());
        double minval = *min_element(all_vals.begin(),all_vals.end());
        
        board<rgba> res;
        for(Point p : iter_all()){
            QColor pcol = col;
            double baseval = pvalfn(m[p]);
            double colored_val = normalize_value(baseval,minval,maxval)*255;
            pcol.setAlpha(colored_val);
            res[p] = color_convert(pcol);
        }
        return res;
    }
};

class GameScreen : public QGraphicsScene
{
public:
    GameScreen(){}
    void keyPressEvent(QKeyEvent * event){
        //this function currently does nothing at all
        //int k = event->key();
        QGraphicsScene::keyPressEvent(event);
    }
    void mousePressEvent(QGraphicsSceneMouseEvent *mouseEvent){
        //this function currently does nothing at all
        QGraphicsScene::mousePressEvent(mouseEvent);
    }
    void draw_map(Map & map,vector<ValueColorer> & colorers){
        this->clear();
        
        for(ValueColorer & vc : colorers){
            if(vc.is_active){
                board<rgba> color_data = vc.color_data(map);
                uchar * data = reinterpret_cast<uchar *>(color_data.Arr.data());
                QImage img(data,WORLD_SIZE,WORLD_SIZE,QImage::Format_RGBA8888);
                
                this->addPixmap(QPixmap::fromImage(img));// = std::unique_ptr<QGraphicsPixmapItem>(this->addPixmap(QPixmap::fromImage(*img)));
            }
        }
    }
};
