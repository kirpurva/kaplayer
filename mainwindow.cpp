#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QFileDialog>
#include <QUrl>
#include <QKeyEvent>

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QStackedLayout>

#include <QGraphicsOpacityEffect>


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
      ui(new Ui::MainWindow),
      mediaPlayer(new QMediaPlayer(this)),
      audioOutput(new QAudioOutput(this)),
      videoWidget(new QVideoWidget(this)),
      hideTimer(new QTimer(this)),
      isPlaying(false),
      isFullScreen(false)
{

    ui->setupUi(this);

    setMouseTracking(true);

    centralWidget()->setMouseTracking(true);

    setWindowTitle("Kaplayer");
    resize(1200,700);



    /*
        MEDIA SETUP
    */


    mediaPlayer->setAudioOutput(audioOutput);
    mediaPlayer->setVideoOutput(videoWidget);


    audioOutput->setVolume(0.5);



    /*
        OVERLAY LAYOUT
    */


    QStackedLayout *stack =
        new QStackedLayout(ui->videoContainer);


    stack->setStackingMode(
        QStackedLayout::StackAll
    );


    stack->addWidget(videoWidget);



    /*
        CONTROL OVERLAY
    */


    controlsOverlay = new QWidget();
    
    controlsOverlay->setObjectName(
        "controlsOverlay"
    );


    QVBoxLayout *overlayRoot =
        new QVBoxLayout(controlsOverlay);

    overlayRoot->setContentsMargins(
    20,
    20,
    20,
    40
    );

    overlayRoot->addStretch();



    QWidget *controlBox =
        new QWidget();


    controlBox->setObjectName(
        "controlBox"
    );
    
    controlBox->setMinimumHeight(90);
    controlBox->setMinimumWidth(700);
    controlBox->setMaximumWidth(900);

    QHBoxLayout *controls =
        new QHBoxLayout(controlBox);



    currentTimeLabel =
        new QLabel("00:00");


    totalTimeLabel =
        new QLabel("00:00");


    positionSlider =
    new QSlider(Qt::Horizontal);

    positionSlider->setMinimumWidth(400);


    volumeSlider =
    new QSlider(Qt::Horizontal);

    volumeSlider->setFixedWidth(120);

    volumeSlider->setRange(
        0,
        100
    );


    volumeSlider->setValue(50);



    openButton =
        new QPushButton("📂");


    playButton =
        new QPushButton("▶");


    fullscreenButton =
        new QPushButton("⛶");
    openButton->setFixedSize(45,45);
    playButton->setFixedSize(45,45);
    fullscreenButton->setFixedSize(45,45);


    controls->addWidget(
        currentTimeLabel
    );


    controls->addWidget(
        positionSlider,
        1
    );


    controls->addWidget(
        totalTimeLabel
    );


    controls->addWidget(
        openButton
    );


    controls->addWidget(
        playButton
    );


    controls->addWidget(
        volumeSlider
    );


    controls->addWidget(
        fullscreenButton
    );



    overlayRoot->addWidget(
        controlBox,
        0,
        Qt::AlignBottom | Qt::AlignHCenter
    );


    stack->addWidget(
        controlsOverlay
    );
    
    stack->setCurrentWidget(
        controlsOverlay
    );

    controlsOverlay->raise();



    /*
        CONNECTIONS
    */


    connect(
        openButton,
        &QPushButton::clicked,
        this,
        &MainWindow::openFile
    );


    connect(
        playButton,
        &QPushButton::clicked,
        this,
        &MainWindow::togglePlayback
    );


    connect(
        fullscreenButton,
        &QPushButton::clicked,
        this,
        &MainWindow::toggleFullScreen
    );


    connect(
        positionSlider,
        &QSlider::sliderMoved,
        this,
        &MainWindow::setPosition
    );


    connect(
        volumeSlider,
        &QSlider::valueChanged,
        this,
        [this](int value)
        {
            audioOutput->setVolume(
                value/100.0
            );
        }
    );



    connect(
        mediaPlayer,
        &QMediaPlayer::positionChanged,
        this,
        &MainWindow::updatePosition
    );


    connect(
        mediaPlayer,
        &QMediaPlayer::durationChanged,
        this,
        &MainWindow::updateDuration
    );



    /*
        FULLSCREEN AUTO HIDE
    */


    hideTimer->setInterval(
        3000
    );


    connect(
    hideTimer,
    &QTimer::timeout,
    this,
    [this]()
    {

        if(isFullScreen)
        {
            controlsOverlay->hide();
        }
        else
        {
            controlsOverlay->show();
        }

    }
    );


    installEventFilter(this);
    setMouseTracking(true);

    ui->videoContainer->installEventFilter(this);
    ui->videoContainer->setMouseTracking(true);

    // Mouse tracking

    setMouseTracking(true);

    ui->videoContainer->setMouseTracking(true);

    videoWidget->setMouseTracking(true);


    // Event filters

    this->installEventFilter(this);

    ui->videoContainer->installEventFilter(this);

    videoWidget->installEventFilter(this);


    /*
        BASIC STYLE
    */


    setStyleSheet(R"(

        QMainWindow {
            background:black;
        }


        #controlBox {

            background-color:
            rgba(20,20,20,180);

            border-radius:20px;

        }


        QPushButton {

            background:transparent;

            color:white;

            border:none;

            font-size:22px;

        }


        QLabel {

            color:white;

        }

    )");


}



MainWindow::~MainWindow()
{
    delete ui;
}





void MainWindow::openFile()
{

    QString file =
        QFileDialog::getOpenFileName(
            this,
            "Open Video",
            "",
            "Videos (*.mp4 *.mkv *.avi)"
        );


    if(file.isEmpty())
        return;


    mediaPlayer->setSource(
        QUrl::fromLocalFile(file)
    );


    mediaPlayer->play();

    isPlaying=true;

    playButton->setText("⏸");

}




void MainWindow::togglePlayback()
{

    if(isPlaying)
    {

        mediaPlayer->pause();

        playButton->setText("▶");

    }

    else
    {

        mediaPlayer->play();

        playButton->setText("⏸");

    }


    isPlaying =
        !isPlaying;

}





void MainWindow::setPosition(int value)
{
    mediaPlayer->setPosition(value);
}





void MainWindow::updatePosition(qint64 value)
{

    positionSlider->setValue(value);


    int sec=value/1000;


    currentTimeLabel->setText(

        QString("%1:%2")
        .arg(sec/60,2,10,QChar('0'))
        .arg(sec%60,2,10,QChar('0'))

    );

}





void MainWindow::updateDuration(qint64 value)
{

    positionSlider->setRange(
        0,
        value
    );


    int sec=value/1000;


    totalTimeLabel->setText(

        QString("%1:%2")
        .arg(sec/60,2,10,QChar('0'))
        .arg(sec%60,2,10,QChar('0'))

    );

}





void MainWindow::toggleFullScreen()
{

    if(isFullScreen)
        showNormal();

    else
        showFullScreen();


    isFullScreen =
        !isFullScreen;


    controlsOverlay->show();

}





void MainWindow::keyPressEvent(
        QKeyEvent *event
)
{

    switch(event->key())
    {


    case Qt::Key_Space:

        togglePlayback();

        break;



    case Qt::Key_F:

        toggleFullScreen();

        break;



    case Qt::Key_Escape:

        if(isFullScreen)
            toggleFullScreen();

        break;



    default:

        QMainWindow::keyPressEvent(event);

    }

}





bool MainWindow::eventFilter(QObject *object, QEvent *event)
{

    if(event->type() == QEvent::MouseMove)
    {
        controlsOverlay->show();


        if(isFullScreen)
        {
            hideTimer->start();
        }
    }


    return QMainWindow::eventFilter(
        object,
        event
    );

}
