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
    void openFile(); // Slot to open a media file
    void play();     // Slot to play the media
    void pause();    // Slot to pause the media
    void stop();     // Slot to stop the media
    void setPosition(int position); // Slot to set the media position
    void updatePosition(qint64 position); // Slot to update the current position
    void updateDuration(qint64 duration); // Slot to update the total duration
    void toggleFullScreen(); // Slot to toggle full-screen mode
    void hideControls();     // Slot to hide the controls
    void showControls();     // Slot to show the controls

private:
    Ui::MainWindow *ui; // Pointer to the UI
    QMediaPlayer *mediaPlayer; // Media player object
    QVideoWidget *videoWidget; // Video widget to display the media
    QTimer *controlHideTimer;  // Timer to hide controls after inactivity
    bool isPlaying;            // Flag to track if media is playing
    bool isFullScreen;         // Flag to track if in full-screen mode

    // Full-screen toolbar controls
    QWidget *fullscreenControlsOverlay; // Full-screen toolbar widget
    QPushButton *fsPlayButton;          // Play button in full-screen mode
    QPushButton *fsPauseButton;         // Pause button in full-screen mode
    QPushButton *fsStopButton;          // Stop button in full-screen mode
    QSlider *fsPositionSlider;          // Position slider in full-screen mode
    QPushButton *fsExitFullscreenButton; // Exit full-screen button
};

#endif // MAINWINDOW_H