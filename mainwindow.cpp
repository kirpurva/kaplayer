#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFileDialog>
#include <QKeyEvent>
#include <QMediaPlayer>
#include <QVideoWidget>
#include <QTimer>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSurfaceFormat>
#include <QMessageBox> 

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , mediaPlayer(new QMediaPlayer(this))
    , videoWidget(new QVideoWidget(this))
    , controlHideTimer(new QTimer(this))
    , isPlaying(false)
    , isFullScreen(false)
{
    
    QSurfaceFormat format;
    format.setVersion(4, 5);  
    format.setProfile(QSurfaceFormat::CoreProfile);
    format.setDepthBufferSize(24);
    format.setStencilBufferSize(8);
    format.setSwapBehavior(QSurfaceFormat::DoubleBuffer);
    QSurfaceFormat::setDefaultFormat(format);

    ui->setupUi(this);
    connect(mediaPlayer, QOverload<QMediaPlayer::Error>::of(&QMediaPlayer::error),
        this, &MainWindow::handleError);
    mediaPlayer->setVideoOutput(videoWidget);
    ui->videoLayout->addWidget(videoWidget);
    connect(mediaPlayer, &QMediaPlayer::mediaStatusChanged, 
            this, &MainWindow::handleMediaStatusChange);

    controlHideTimer->setInterval(3000);
    connect(controlHideTimer, &QTimer::timeout, this, &MainWindow::hideControls);

    videoWidget->setMouseTracking(true);
    videoWidget->installEventFilter(this);

    createFloatingToolbar();

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
}

MainWindow::~MainWindow()
{
    delete mediaPlayer;
    delete videoWidget;
    delete ui;
}

void MainWindow::createFloatingToolbar()
{

    floatingToolbar = new QWidget(this);
    floatingToolbar->setWindowFlags(Qt::FramelessWindowHint | Qt::Tool | Qt::WindowStaysOnTopHint);
    floatingToolbar->setStyleSheet("background: rgba(0, 0, 0, 150); border-radius: 5px;");


    QHBoxLayout *layout = new QHBoxLayout(floatingToolbar);
    layout->setContentsMargins(10, 5, 10, 5);


    fsPlayButton = new QPushButton("Play", floatingToolbar);
    fsPauseButton = new QPushButton("Pause", floatingToolbar);
    fsStopButton = new QPushButton("Stop", floatingToolbar);
    fsPositionSlider = new QSlider(Qt::Horizontal, floatingToolbar);
    fsExitFullscreenButton = new QPushButton("Exit Fullscreen", floatingToolbar);

    layout->addWidget(fsPlayButton);
    layout->addWidget(fsPauseButton);
    layout->addWidget(fsStopButton);
    layout->addWidget(fsPositionSlider);
    layout->addWidget(fsExitFullscreenButton);

    connect(fsPlayButton, &QPushButton::clicked, this, &MainWindow::play);
    connect(fsPauseButton, &QPushButton::clicked, this, &MainWindow::pause);
    connect(fsStopButton, &QPushButton::clicked, this, &MainWindow::stop);
    connect(fsPositionSlider, &QSlider::sliderMoved, this, &MainWindow::setPosition);
    connect(fsExitFullscreenButton, &QPushButton::clicked, this, &MainWindow::toggleFullScreen);


    floatingToolbar->hide();
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
    if (isFullScreen) {
        floatingToolbar->hide();
    }
}

void MainWindow::showControls()
{
    if (isFullScreen) {
        floatingToolbar->move((width() - floatingToolbar->width()) / 2, height() - floatingToolbar->height() - 10);
        floatingToolbar->show();
        controlHideTimer->start(3000); 
    }
}

void MainWindow::toggleFullScreen()
{
    if (isFullScreen) {
        showNormal();
        ui->controlsOverlay->setVisible(true);
        floatingToolbar->hide();
    } else {
        showFullScreen();
        ui->controlsOverlay->setVisible(false);
        showControls(); 
    }
    isFullScreen = !isFullScreen;
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

void MainWindow::handleError(QMediaPlayer::Error error)
{
    qDebug() << "Media Player Error:" << error;
    qDebug() << "Detailed Error:" << mediaPlayer->errorString();
    
    QString errorMsg = "Unknown media error";
    switch(error) {
        case QMediaPlayer::ResourceError:
            errorMsg = "Resource Error: Could not load media. Check GStreamer plugins.";
            break;
        case QMediaPlayer::FormatError:
            errorMsg = "Format Error: Unsupported media format.";
            break;
        case QMediaPlayer::NetworkError:
            errorMsg = "Network Error: Cannot access media source.";
            break;
        default:
            break;
    }
    
    QMessageBox::warning(this, "Media Player Error", errorMsg);
}

void MainWindow::handleMediaStatusChange(QMediaPlayer::MediaStatus status)
{
    qDebug() << "Media Status:" << status;
    
    switch(status) {
        case QMediaPlayer::LoadingMedia:
            qDebug() << "Loading media...";
            break;
        case QMediaPlayer::LoadedMedia:
            qDebug() << "Media loaded successfully";
            break;
        case QMediaPlayer::InvalidMedia:
            qDebug() << "Invalid media";
            QMessageBox::warning(this, "Media Error", 
                "Cannot play this media file. Check file format and codecs.");
            break;
        default:
            break;
    }
}