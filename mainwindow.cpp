#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFileDialog>
#include <QUrl>
#include <QKeyEvent>
#include <QDebug>



MainWindow::MainWindow(QWidget *parent)
    :
      QMainWindow(parent),
      ui(new Ui::MainWindow)
{

    ui->setupUi(this);
    qDebug() << "NEW KAPLAYER UI BUILD RUNNING";


    resize(1200,700);



    // Video


    player = new QMediaPlayer(this);

    audio = new QAudioOutput(this);

    video = new QVideoWidget(ui->videoContainer);



    player->setAudioOutput(audio);

    player->setVideoOutput(video);



    video->setParent(
    ui->videoContainer
    );


    video->setGeometry(
    ui->videoContainer->rect()
    );


    video->show();



    buildToolbar();



    hideTimer =
        new QTimer(this);


    hideTimer->setInterval(5000);


    connect(
        hideTimer,
        &QTimer::timeout,
        this,
        [this]()
        {

            if(fullscreen)

                toolbar->hide();

        }
    );



    connect(player,
            &QMediaPlayer::positionChanged,
            this,
            &MainWindow::updatePosition);


    connect(player,
            &QMediaPlayer::durationChanged,
            this,
            &MainWindow::updateDuration);



    video->installEventFilter(this);

    ui->videoContainer->installEventFilter(this);

    installEventFilter(this);



    setMouseTracking(true);

    video->setMouseTracking(true);

    ui->videoContainer->setMouseTracking(true);



    setStyleSheet(R"(

        QMainWindow {
            background:black;
        }


        #toolbar {

            background:
            rgba(20,20,20,190);

            border-radius:25px;

        }


        QPushButton {

            color:white;

            background:none;

            border:none;

            font-size:20px;

        }


        QLabel {

            color:white;

        }


    )");

    QTimer::singleShot(
    0,
    this,
    [this]()
    {

        video->setGeometry(
            ui->videoContainer->rect()
        );


        positionToolbar();


        toolbar->show();

        toolbar->raise();

    }
);

}



void MainWindow::buildToolbar()
{

    toolbar =
        new QWidget(ui->videoContainer);


    toolbar->setObjectName("toolbar");



    auto row =
        new QHBoxLayout(toolbar);



    currentLabel =
        new QLabel("00:00");


    durationLabel =
        new QLabel("00:00");


    seekSlider =
        new QSlider(Qt::Horizontal);


    volumeSlider =
        new QSlider(Qt::Horizontal);


    volumeSlider->setFixedWidth(120);



    openBtn =
        new QPushButton("📂");


    playBtn =
        new QPushButton("▶");


    fullBtn =
        new QPushButton("⛶");



    row->addWidget(currentLabel);

    row->addWidget(seekSlider,1);

    row->addWidget(durationLabel);

    row->addWidget(openBtn);

    row->addWidget(playBtn);

    row->addWidget(volumeSlider);

    row->addWidget(fullBtn);



    connect(openBtn,
            &QPushButton::clicked,
            this,
            &MainWindow::openFile);



    connect(playBtn,
            &QPushButton::clicked,
            this,
            &MainWindow::togglePlayback);



    connect(fullBtn,
            &QPushButton::clicked,
            this,
            &MainWindow::toggleFullScreen);



    toolbar->show();

    toolbar->raise();

}



void MainWindow::positionToolbar()
{

    int w =
        qMin(width()-80,850);


    int h=70;


    int x =
        (ui->videoContainer->width()-w)/2;


    int y =
        ui->videoContainer->height()
        - h
        - 30;



    toolbar->setGeometry(
        x,y,w,h
    );


    toolbar->raise();

}





void MainWindow::resizeEvent(QResizeEvent *e)
{

    QMainWindow::resizeEvent(e);


    video->setGeometry(
        ui->videoContainer->rect()
    );


    positionToolbar();


    toolbar->raise();

}





bool MainWindow::eventFilter(
        QObject *,
        QEvent *event)
{

    if(event->type()==QEvent::MouseMove)
    {

        toolbar->show();

        toolbar->raise();



        if(fullscreen)

            hideTimer->start();

    }


    return false;

}




void MainWindow::openFile()
{

    QString f =
        QFileDialog::getOpenFileName(
            this,
            "Open video"
        );


    if(f.isEmpty())
        return;



    player->setSource(
        QUrl::fromLocalFile(f)
    );


    player->play();

    toolbar->show();

    toolbar->raise();

    positionToolbar();


    playing=true;


    playBtn->setText("⏸");


    toolbar->show();

    toolbar->raise();

}





void MainWindow::togglePlayback()
{

    if(playing)
    {
        player->pause();

        playBtn->setText("▶");
    }

    else
    {
        player->play();

        playBtn->setText("⏸");
    }


    playing=!playing;

}




void MainWindow::toggleFullScreen()
{

    fullscreen =
        !fullscreen;


    if(fullscreen)

        showFullScreen();


    else
    {

        showNormal();

        toolbar->show();

    }


}




void MainWindow::updatePosition(qint64 p)
{

    seekSlider->setValue(p);

}




void MainWindow::updateDuration(qint64 d)
{

    seekSlider->setRange(
        0,d
    );

}





void MainWindow::keyPressEvent(QKeyEvent *e)
{

    if(e->key()==Qt::Key_Escape
            && fullscreen)

        toggleFullScreen();

}




MainWindow::~MainWindow()
{

    delete ui;

}
