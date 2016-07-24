#include <QMainWindow>
#include <functional>
#include <QCheckBox>
#include <QTimer>
using namespace std;

class QGraphicsView;
class GameScreen;
struct check_obj;

constexpr size_t NUM_DISPLAYS = 5;
namespace Ui {
class MainWindow;
}

struct check_obj{
    function<void()> func;
    QCheckBox * box;
    check_obj(){
        box = nullptr;
    }
    check_obj(string str,bool def_check,function<void()> infunc,QWidget * parent=nullptr){
        func = infunc;
        box = new QCheckBox(QString(str.c_str()),parent);
        box->setChecked(def_check);
    }
    void call_if_checked(){
        if(box->isChecked()){
            func();
        }
    }
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    
    void draw();
protected:
    Ui::MainWindow *ui;
    QGraphicsView * veiw;
    GameScreen * screen;
    check_obj chks[NUM_DISPLAYS];
    QTimer frame_timer;
    QWidget * make_layout();
};
