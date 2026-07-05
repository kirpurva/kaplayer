#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QAudio>
#include <QAudioOutput>
#include <QVideoWidget>
#include <QPushButton>
#include <QSlider>
#include <QLabel>
#include <QTimer>
#include <QHBoxLayout>
#include <QFileDialog>
#include <QStandardPaths>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QStyle>
#include <QTime>
#include <QUrl>

namespace {
constexpr int kToolbarMaxWidth   = 850;
constexpr int kToolbarHeight     = 64;
constexpr int kToolbarSideMargin = 40;
constexpr int kToolbarBottomGap  = 24;
constexpr int kHideDelayMs       = 3000;
constexpr int kDefaultVolume     = 80;  // percent
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
      ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    resize(1200, 700);

    // ---- Media pipeline ----
    player = new QMediaPlayer(this);
    audio  = new QAudioOutput(this);
    video  = new QVideoWidget(ui->videoContainer);

    player->setAudioOutput(audio);
    player->setVideoOutput(video);
    video->setGeometry(ui->videoContainer->rect());
    video->show();

    buildToolbar();

    // ---- Auto-hide: same rule in windowed and fullscreen mode ----
    hideTimer = new QTimer(this);
    hideTimer->setSingleShot(true);
    hideTimer->setInterval(kHideDelayMs);
    connect(hideTimer, &QTimer::timeout, this, [this]() {
        // Only hide while actually playing; keep controls up when paused.
        if (player->playbackState() == QMediaPlayer::PlayingState) {
            toolbar->hide();
            if (fullscreen)
                video->setCursor(Qt::BlankCursor);
        }
    });

    // ---- Player state -> UI (single source of truth) ----
    connect(player, &QMediaPlayer::positionChanged,
            this, &MainWindow::updatePosition);
    connect(player, &QMediaPlayer::durationChanged,
            this, &MainWindow::updateDuration);
    connect(player, &QMediaPlayer::playbackStateChanged,
            this, &MainWindow::onPlaybackStateChanged);

    // ---- Mouse tracking to wake the toolbar ----
    video->installEventFilter(this);
    ui->videoContainer->installEventFilter(this);
    setMouseTracking(true);
    video->setMouseTracking(true);
    ui->videoContainer->setMouseTracking(true);

    // Defer initial layout until geometry is final.
    QTimer::singleShot(0, this, [this]() {
        video->setGeometry(ui->videoContainer->rect());
        positionToolbar();
        toolbar->show();
        toolbar->raise();
    });
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::buildToolbar()
{
    toolbar = new QWidget(ui->videoContainer);
    toolbar->setObjectName("toolbar");

    const int iconPx = 22;
    const QSize iconSize(iconPx, iconPx);

    // Standard theme icons render identically across platforms,
    // and play/pause occupy the same width (no layout shift).
    openBtn = new QPushButton;
    openBtn->setIcon(style()->standardIcon(QStyle::SP_DialogOpenButton));
    openBtn->setIconSize(iconSize);
    openBtn->setToolTip(tr("Open video (Ctrl+O)"));

    playBtn = new QPushButton;
    playBtn->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));
    playBtn->setIconSize(iconSize);
    playBtn->setToolTip(tr("Play/Pause (Space)"));

    fullBtn = new QPushButton;
    fullBtn->setIcon(style()->standardIcon(QStyle::SP_TitleBarMaxButton));
    fullBtn->setIconSize(iconSize);
    fullBtn->setToolTip(tr("Fullscreen (F)"));

    currentLabel  = new QLabel(QStringLiteral("00:00"));
    durationLabel = new QLabel(QStringLiteral("00:00"));

    seekSlider = new QSlider(Qt::Horizontal);
    seekSlider->setToolTip(tr("Seek"));

    volumeSlider = new QSlider(Qt::Horizontal);
    volumeSlider->setRange(0, 100);
    volumeSlider->setValue(kDefaultVolume);
    volumeSlider->setFixedWidth(110);
    volumeSlider->setToolTip(tr("Volume"));

    // Grouping: file action | transport | timeline | volume | window
    auto *row = new QHBoxLayout(toolbar);
    row->setContentsMargins(16, 8, 16, 8);
    row->setSpacing(10);
    row->addWidget(openBtn);
    row->addWidget(playBtn);
    row->addWidget(currentLabel);
    row->addWidget(seekSlider, /*stretch*/ 1);
    row->addWidget(durationLabel);
    row->addWidget(volumeSlider);
    row->addWidget(fullBtn);

    connect(openBtn, &QPushButton::clicked, this, &MainWindow::openFile);
    connect(playBtn, &QPushButton::clicked, this, &MainWindow::togglePlayback);
    connect(fullBtn, &QPushButton::clicked, this, &MainWindow::toggleFullScreen);

    // Seeking: user drag drives the player; live jumps while scrubbing.
    connect(seekSlider, &QSlider::sliderMoved,
            player, &QMediaPlayer::setPosition);

    // Volume: perceptual (logarithmic) mapping so the slider feels linear.
    auto applyVolume = [this](int value) {
        audio->setVolume(QAudio::convertVolume(
            value / 100.0,
            QAudio::LogarithmicVolumeScale,
            QAudio::LinearVolumeScale));
    };
    connect(volumeSlider, &QSlider::valueChanged, this, applyVolume);
    applyVolume(kDefaultVolume);
}

void MainWindow::positionToolbar()
{
    // All geometry derives from videoContainer — one coordinate base.
    const int cw = ui->videoContainer->width();
    const int ch = ui->videoContainer->height();

    const int w = qMin(cw - 2 * kToolbarSideMargin, kToolbarMaxWidth);
    const int x = (cw - w) / 2;
    const int y = ch - kToolbarHeight - kToolbarBottomGap;

    toolbar->setGeometry(x, y, w, kToolbarHeight);
    toolbar->raise();
}

void MainWindow::wakeToolbar()
{
    toolbar->show();
    toolbar->raise();
    video->setCursor(Qt::ArrowCursor);
    hideTimer->start();
}

void MainWindow::resizeEvent(QResizeEvent *event)
{
    QMainWindow::resizeEvent(event);
    video->setGeometry(ui->videoContainer->rect());
    positionToolbar();
}

bool MainWindow::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::MouseMove) {
        wakeToolbar();
    } else if (event->type() == QEvent::MouseButtonDblClick) {
        toggleFullScreen();
        return true;
    }
    return QMainWindow::eventFilter(obj, event);
}

void MainWindow::keyPressEvent(QKeyEvent *event)
{
    switch (event->key()) {
    case Qt::Key_Space:
        togglePlayback();
        return;
    case Qt::Key_F:
        toggleFullScreen();
        return;
    case Qt::Key_O:
        if (event->modifiers() & Qt::ControlModifier) {
            openFile();
            return;
        }
        break;
    case Qt::Key_Escape:
        if (fullscreen) {
            toggleFullScreen();
            return;
        }
        break;
    default:
        break;
    }
    QMainWindow::keyPressEvent(event);
}

void MainWindow::openFile()
{
    const QString filter =
        tr("Videos (*.mp4 *.mkv *.avi *.mov *.webm);;All files (*)");
    const QString file = QFileDialog::getOpenFileName(
        this, tr("Open video"),
        QStandardPaths::writableLocation(QStandardPaths::MoviesLocation),
        filter);

    if (file.isEmpty())
        return;

    player->setSource(QUrl::fromLocalFile(file));
    player->play();
    wakeToolbar();
}

void MainWindow::togglePlayback()
{
    if (player->playbackState() == QMediaPlayer::PlayingState)
        player->pause();
    else
        player->play();
    // Button icon updates via onPlaybackStateChanged — no flag to drift.
}

void MainWindow::toggleFullScreen()
{
    fullscreen = !fullscreen;
    if (fullscreen) {
        showFullScreen();
    } else {
        showNormal();
        video->setCursor(Qt::ArrowCursor);
    }
    wakeToolbar();
}

void MainWindow::updatePosition(qint64 pos)
{
    // Don't fight the user mid-scrub.
    if (!seekSlider->isSliderDown())
        seekSlider->setValue(static_cast<int>(pos));
    currentLabel->setText(formatTime(pos));
}

void MainWindow::updateDuration(qint64 duration)
{
    seekSlider->setRange(0, static_cast<int>(duration));
    durationLabel->setText(formatTime(duration));
}

void MainWindow::onPlaybackStateChanged(QMediaPlayer::PlaybackState state)
{
    const bool isPlaying = (state == QMediaPlayer::PlayingState);
    playBtn->setIcon(style()->standardIcon(
        isPlaying ? QStyle::SP_MediaPause : QStyle::SP_MediaPlay));
    if (!isPlaying)
        wakeToolbar();  // media ended or paused: keep controls visible
}

QString MainWindow::formatTime(qint64 ms)
{
    const qint64 totalSecs = ms / 1000;
    const qint64 h = totalSecs / 3600;
    const qint64 m = (totalSecs % 3600) / 60;
    const qint64 s = totalSecs % 60;

    if (h > 0)
        return QStringLiteral("%1:%2:%3")
            .arg(h)
            .arg(m, 2, 10, QLatin1Char('0'))
            .arg(s, 2, 10, QLatin1Char('0'));
    return QStringLiteral("%1:%2")
        .arg(m, 2, 10, QLatin1Char('0'))
        .arg(s, 2, 10, QLatin1Char('0'));
}
