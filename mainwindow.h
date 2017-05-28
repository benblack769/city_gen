#include <QMainWindow>
#include <QTimer>
#include <world.h>

class QGraphicsView;
class GameScreen;
class QCheckBox;
struct check_obj;

constexpr size_t NUM_DISPLAYS = 1;
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
    QCheckBox * chks[NUM_DISPLAYS];
    QTimer frame_timer;
    QTimer update_timer;
    QWidget * make_layout();
};
