#ifndef MAINWINDOW_H
#define MAINWINDOW_H


#include <QMainWindow>

#include <QMediaPlayer>
#include <QAudioOutput>
#include <QVideoWidget>

#include <QPushButton>
#include <QSlider>
#include <QLabel>

#include <QTimer>
#include <QResizeEvent>


QT_BEGIN_NAMESPACE
namespace Ui
{
class MainWindow;
}
QT_END_NAMESPACE



class MainWindow : public QMainWindow
{

    Q_OBJECT


public:

    MainWindow(QWidget *parent=nullptr);

    ~MainWindow();



protected:

    void resizeEvent(QResizeEvent *event) override;

    bool eventFilter(QObject *obj,QEvent *event) override;

    void keyPressEvent(QKeyEvent *event) override;



private slots:


    void openFile();

    void togglePlayback();

    void toggleFullScreen();


    void updatePosition(qint64 pos);

    void updateDuration(qint64 duration);



private:


    Ui::MainWindow *ui;


    // Media

    QMediaPlayer *player;

    QAudioOutput *audio;

    QVideoWidget *video;



    // Floating toolbar

    QWidget *toolbar;


    QPushButton *openBtn;

    QPushButton *playBtn;

    QPushButton *fullBtn;


    QSlider *seekSlider;

    QSlider *volumeSlider;


    QLabel *currentLabel;

    QLabel *durationLabel;



    QTimer *hideTimer;


    bool fullscreen=false;

    bool playing=false;



    void buildToolbar();

    void positionToolbar();

};


#endif
