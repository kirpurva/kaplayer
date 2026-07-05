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
#include <QFileInfo>
#include <QMessageBox>
#include <QProxyStyle>
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

// Makes a left-click on a slider groove jump the handle to that spot and
// begin a drag, so clicking and dragging seek identically (no page-step
// jump that then snaps back).
class JumpSliderStyle : public QProxyStyle {
public:
    using QProxyStyle::QProxyStyle;
    int styleHint(QStyle::StyleHint hint, const QStyleOption *option,
                  const QWidget *widget,
                  QStyleHintReturn *returnData) const override
    {
        if (hint == QStyle::SH_Slider_AbsoluteSetButtons)
            return Qt::LeftButton;
        return QProxyStyle::styleHint(hint, option, widget, returnData);
    }
};
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
        if (player->playbackState() != QMediaPlayer::PlayingState)
            return;
        // Never hide the controls out from under the cursor or mid-drag.
        if (toolbar->underMouse() || seekSlider->isSliderDown()) {
            hideTimer->start();
            return;
        }
        toolbar->hide();
        if (isFullScreen())
            video->setCursor(Qt::BlankCursor);
    });

    // ---- Player state -> UI (single source of truth) ----
    connect(player, &QMediaPlayer::positionChanged,
            this, &MainWindow::updatePosition);
    connect(player, &QMediaPlayer::durationChanged,
            this, &MainWindow::updateDuration);
    connect(player, &QMediaPlayer::playbackStateChanged,
            this, &MainWindow::onPlaybackStateChanged);
    connect(player, &QMediaPlayer::errorOccurred,
            this, [this](QMediaPlayer::Error, const QString &errorString) {
        QMessageBox::warning(this, tr("Kaplayer"),
                             errorString.isEmpty()
                                 ? tr("This file could not be played.")
                                 : errorString);
    });

    // ---- Mouse tracking to wake the toolbar ----
    // The videoContainer filter also drives geometry: its Resize events
    // arrive after layout, so video/toolbar placement is never stale.
    video->installEventFilter(this);
    ui->videoContainer->installEventFilter(this);
    setMouseTracking(true);
    video->setMouseTracking(true);
    ui->videoContainer->setMouseTracking(true);
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

    // Click-to-seek: groove clicks jump-and-drag on both sliders.
    auto *jumpStyle = new JumpSliderStyle;
    jumpStyle->setParent(this);
    seekSlider->setStyle(jumpStyle);
    volumeSlider->setStyle(jumpStyle);

    // Shortcuts stay global (no control may steal keyboard focus), and
    // hovering any part of the toolbar counts as activity so the
    // auto-hide never pulls the controls out from under the cursor.
    toolbar->setMouseTracking(true);
    toolbar->installEventFilter(this);
    const auto children = toolbar->findChildren<QWidget *>();
    for (QWidget *child : children) {
        child->setFocusPolicy(Qt::NoFocus);
        child->setMouseTracking(true);
        child->installEventFilter(this);
    }
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

bool MainWindow::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == ui->videoContainer && event->type() == QEvent::Resize) {
        video->setGeometry(ui->videoContainer->rect());
        positionToolbar();
    } else if (event->type() == QEvent::MouseMove) {
        wakeToolbar();
    } else if (event->type() == QEvent::MouseButtonDblClick
               && (obj == video || obj == ui->videoContainer)) {
        toggleFullScreen();
        return true;
    }
    return QMainWindow::eventFilter(obj, event);
}

void MainWindow::changeEvent(QEvent *event)
{
    QMainWindow::changeEvent(event);
    if (event->type() == QEvent::WindowStateChange && fullBtn) {
        // Mirror window state on the button icon (same pattern as
        // play/pause) and never leave a blanked cursor behind.
        fullBtn->setIcon(style()->standardIcon(
            isFullScreen() ? QStyle::SP_TitleBarNormalButton
                           : QStyle::SP_TitleBarMaxButton));
        if (!isFullScreen())
            video->setCursor(Qt::ArrowCursor);
    }
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
        if (isFullScreen()) {
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
    setWindowTitle(QStringLiteral("%1 — Kaplayer")
                       .arg(QFileInfo(file).fileName()));
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
    // Window state is the single source of truth — no shadow flag to drift.
    // Icon and cursor restoration happen in changeEvent, so they also
    // track state changes made by the OS or window manager.
    if (isFullScreen())
        showNormal();
    else
        showFullScreen();
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
    // Playing re-arms the auto-hide countdown; paused/stopped shows the
    // controls and the timer callback then declines to hide them.
    wakeToolbar();
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
