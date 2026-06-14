#ifndef MAINWINDOW_H
#define MAINWINDOW_H


#include <QMainWindow>

#include <QMediaPlayer>
#include <QAudioOutput>
#include <QVideoWidget>

#include <QSlider>
#include <QKeyEvent>
#include <QStackedLayout>
#include <QPushButton>
#include <QSlider>
#include <QLabel>
#include <QTimer>
#include <QStackedLayout>
#include <QEvent>

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

    explicit MainWindow(QWidget *parent = nullptr);

    ~MainWindow();



protected:

    void keyPressEvent(QKeyEvent *event) override;
    bool eventFilter(QObject *object, QEvent *event) override;


private slots:


    // File

    void openFile();



    // Playback

    void togglePlayback();



    // Controls

    void setPosition(int position);

    void updatePosition(qint64 position);

    void updateDuration(qint64 duration);



    // Window

    void toggleFullScreen();



private:


    Ui::MainWindow *ui;


    // Media engine

    QMediaPlayer *mediaPlayer;

    QAudioOutput *audioOutput;

    QVideoWidget *videoWidget;

    QStackedLayout *overlayLayout;

    // Controls

    QWidget *controlsOverlay;

    QPushButton *openButton;
    QPushButton *playButton;
    QPushButton *fullscreenButton;

    QSlider *positionSlider;
    QSlider *volumeSlider;

    QLabel *currentTimeLabel;
    QLabel *totalTimeLabel;

    QTimer *hideTimer;

    // States

    bool isPlaying;

    bool isFullScreen;

};


#endif

