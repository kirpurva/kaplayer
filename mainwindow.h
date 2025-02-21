#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMediaPlayer>
#include <QVideoWidget>
#include <QSlider>
#include <QTimer>  
#include <QPushButton> 

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void keyPressEvent(QKeyEvent *event) override;
    bool eventFilter(QObject *obj, QEvent *event) override; 

private slots:
    void openFile(); 
    void play();     
    void pause();    
    void stop();     
    void setPosition(int position); 
    void updatePosition(qint64 position); 
    void updateDuration(qint64 duration); 
    void toggleFullScreen(); 
    void hideControls();    
    void showControls();     

private:
    Ui::MainWindow *ui; 
    QMediaPlayer *mediaPlayer; 
    QVideoWidget *videoWidget; 
    QTimer *controlHideTimer;  
    bool isPlaying;            
    bool isFullScreen;         

   
    QWidget *fullscreenControlsOverlay;  
    QPushButton *fsPlayButton;          
    QPushButton *fsPauseButton;        
    QPushButton *fsStopButton;          
    QSlider *fsPositionSlider;          
    QPushButton *fsExitFullscreenButton; 
};

#endif 