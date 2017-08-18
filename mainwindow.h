#pragma once
#include <QMainWindow>
#include <QTimer>
#include <QCheckBox>
#include <world.h>
#include <gamescreen.h>

class QGraphicsView;
class GameScreen;
class QCheckBox;
struct check_obj;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    
    void draw();
protected:
    World world;
    Ui::MainWindow *ui;
    QGraphicsView * veiw;
    GameScreen * screen;
    vector<QCheckBox *> chks;
    vector<ValueColorer> colorers;
    QTimer frame_timer;
    QTimer update_timer;
    QWidget * make_layout();
    void add_checkbox(QString str, QWidget * parent, bool is_checked,QColor col,function<double(PointProperty)> colorer_fn){
        QCheckBox * box = new QCheckBox(str,parent);
        box->setChecked(is_checked);
        connect(box,QCheckBox::stateChanged,this,draw);
        chks.push_back(box);
        
        int my_cc_loc = colorers.size();
        colorers.push_back(ValueColorer(is_checked,col,colorer_fn));
        connect(box,QCheckBox::stateChanged,[this,my_cc_loc](){
            this->colorers[my_cc_loc].is_active = !this->colorers[my_cc_loc].is_active;
        });
    }
};
