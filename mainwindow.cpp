#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFileDialog>
#include <QKeyEvent>
#include <QMediaPlayer>
#include <QVideoWidget>
#include <QTimer>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , mediaPlayer(new QMediaPlayer(this))
    , videoWidget(new QVideoWidget(this))
    , controlHideTimer(new QTimer(this))
    , isPlaying(false)
    , isFullScreen(false)
{
    ui->setupUi(this);

   
    mediaPlayer->setVideoOutput(videoWidget);
    ui->videoLayout->addWidget(videoWidget);

   
    controlHideTimer->setInterval(3000);
    connect(controlHideTimer, &QTimer::timeout, this, &MainWindow::hideControls);

   
    videoWidget->setMouseTracking(true);
    videoWidget->installEventFilter(this);

   
    fullscreenControlsOverlay = ui->fullscreenControlsOverlay;
    fsPlayButton = ui->fsPlayButton;
    fsPauseButton = ui->fsPauseButton;
    fsStopButton = ui->fsStopButton;
    fsPositionSlider = ui->fsPositionSlider;
    fsExitFullscreenButton = ui->fsExitFullscreenButton;

   
    connect(fsPlayButton, &QPushButton::clicked, this, &MainWindow::play);
    connect(fsPauseButton, &QPushButton::clicked, this, &MainWindow::pause);
    connect(fsStopButton, &QPushButton::clicked, this, &MainWindow::stop);
    connect(fsPositionSlider, &QSlider::sliderMoved, this, &MainWindow::setPosition);
    connect(fsExitFullscreenButton, &QPushButton::clicked, this, &MainWindow::toggleFullScreen);

   
    connect(ui->openButton, &QPushButton::clicked, this, &MainWindow::openFile);
    connect(ui->playButton, &QPushButton::clicked, this, &MainWindow::play);
    connect(ui->pauseButton, &QPushButton::clicked, this, &MainWindow::pause);
    connect(ui->stopButton, &QPushButton::clicked, this, &MainWindow::stop);
    connect(ui->fullscreenButton, &QPushButton::clicked, this, &MainWindow::toggleFullScreen);
    connect(ui->positionSlider, &QSlider::sliderMoved, this, &MainWindow::setPosition);
    connect(ui->volumeSlider, &QSlider::valueChanged, mediaPlayer, &QMediaPlayer::setVolume);

   
    ui->volumeSlider->setValue(50);
    mediaPlayer->setVolume(50);

    
    connect(mediaPlayer, &QMediaPlayer::positionChanged, this, &MainWindow::updatePosition);
    connect(mediaPlayer, &QMediaPlayer::durationChanged, this, &MainWindow::updateDuration);

   
    ui->pauseButton->setEnabled(false);
    ui->controlsOverlay->setAttribute(Qt::WA_TranslucentBackground);
    ui->controlsOverlay->raise();
    
    
    fullscreenControlsOverlay->raise();
    fullscreenControlsOverlay->setVisible(false);
}

MainWindow::~MainWindow()
{
    delete mediaPlayer;
    delete videoWidget;
    delete ui;
}

void MainWindow::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_F11 || event->key() == Qt::Key_F) {
        toggleFullScreen();
    } else if (event->key() == Qt::Key_Escape && isFullScreen) {
        toggleFullScreen();
    } else {
        QMainWindow::keyPressEvent(event);
    }
}

void MainWindow::hideControls()
{
    if (isFullScreen && fullscreenControlsOverlay->isVisible()) {
        fullscreenControlsOverlay->setVisible(false);
    }
}

void MainWindow::showControls()
{
    if (isFullScreen) {
        fullscreenControlsOverlay->setVisible(true);
        controlHideTimer->start(3000); 
    }
}

void MainWindow::toggleFullScreen()
{
    if (isFullScreen) {
        showNormal();
        ui->controlsOverlay->setVisible(true);
        fullscreenControlsOverlay->setVisible(false);
    } else {
        showFullScreen();
        ui->controlsOverlay->setVisible(false);
        fullscreenControlsOverlay->setVisible(true);
    }
    isFullScreen = !isFullScreen;
    controlHideTimer->start();
}

void MainWindow::openFile()
{
    QString fileName = QFileDialog::getOpenFileName(this, "Open Video File", "", "Videos (*.mp4 *.avi *.mkv)");
    if (!fileName.isEmpty()) {
        mediaPlayer->setMedia(QUrl::fromLocalFile(fileName));
        play();
    }
}

void MainWindow::play()
{
    mediaPlayer->play();
    isPlaying = true;
    ui->playButton->setEnabled(false);
    ui->pauseButton->setEnabled(true);
    fsPlayButton->setEnabled(false);
    fsPauseButton->setEnabled(true);
}

void MainWindow::pause()
{
    mediaPlayer->pause();
    isPlaying = false;
    ui->playButton->setEnabled(true);
    ui->pauseButton->setEnabled(false);
    fsPlayButton->setEnabled(true);
    fsPauseButton->setEnabled(false);
}

void MainWindow::stop()
{
    mediaPlayer->stop();
    isPlaying = false;
    ui->playButton->setEnabled(true);
    ui->pauseButton->setEnabled(false);
    fsPlayButton->setEnabled(true);
    fsPauseButton->setEnabled(false);
}

void MainWindow::setPosition(int position)
{
    mediaPlayer->setPosition(position);
}

void MainWindow::updatePosition(qint64 position)
{
    ui->positionSlider->setValue(position);
    fsPositionSlider->setValue(position); 
    qint64 seconds = position / 1000;
    QString time = QString("%1:%2").arg(seconds / 60, 2, 10, QChar('0')).arg(seconds % 60, 2, 10, QChar('0'));
    ui->currentTimeLabel->setText(time);
}

void MainWindow::updateDuration(qint64 duration)
{
    ui->positionSlider->setRange(0, duration);
    fsPositionSlider->setRange(0, duration); 
    qint64 seconds = duration / 1000;
    QString time = QString("%1:%2").arg(seconds / 60, 2, 10, QChar('0')).arg(seconds % 60, 2, 10, QChar('0'));
    ui->totalTimeLabel->setText(time);
}

bool MainWindow::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == videoWidget) {
        if (event->type() == QEvent::MouseMove) {
            showControls();
            return true;
        }
    }
    return QMainWindow::eventFilter(obj, event);
}