#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QIcon>
#include <QMainWindow>
#include <QMediaPlayer>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class QAudioOutput;
class QVideoWidget;
class QPushButton;
class QSlider;
class QLabel;
class QTimer;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void showEvent(QShowEvent *event) override;
    void changeEvent(QEvent *event) override;
    bool eventFilter(QObject *obj, QEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;

private slots:
    void openFile();
    void togglePlayback();
    void toggleFullScreen();
    void updatePosition(qint64 pos);
    void updateDuration(qint64 duration);
    void onPlaybackStateChanged(QMediaPlayer::PlaybackState state);

private:
    Ui::MainWindow *ui;

    // Media
    QMediaPlayer *player = nullptr;
    QAudioOutput *audio = nullptr;
    QVideoWidget *video = nullptr;

    // Floating toolbar
    QWidget *toolbar = nullptr;
    QPushButton *openBtn = nullptr;
    QPushButton *playBtn = nullptr;
    QPushButton *fullBtn = nullptr;
    QSlider *seekSlider = nullptr;
    QSlider *volumeSlider = nullptr;
    QLabel *currentLabel = nullptr;
    QLabel *durationLabel = nullptr;

    QTimer *hideTimer = nullptr;
    qint64 pendingSeek = -1;  // seek target awaiting backend confirmation

    // Themed toolbar icons (SVG, from resources)
    QIcon playIcon;
    QIcon pauseIcon;
    QIcon fullscreenIcon;
    QIcon fullscreenExitIcon;

    void buildToolbar();
    void positionToolbar();
    void wakeToolbar();
    void seekTo(qint64 pos);
    static QString formatTime(qint64 ms);
};

#endif // MAINWINDOW_H
