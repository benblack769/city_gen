#include <QMainWindow>
#include <QTimer>
#include "check_obj.h"

class QGraphicsView;
class GameScreen;
struct check_obj;

constexpr size_t NUM_DISPLAYS = 4;
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
    Ui::MainWindow *ui;
    QGraphicsView * veiw;
    GameScreen * screen;
    check_obj chks[NUM_DISPLAYS];
    QTimer frame_timer;
    QWidget * make_layout();
};
