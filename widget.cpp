// widget.cpp
#include "widget.h"
#include "ui_widget.h"
#include <QDebug>

const int CAP_SIZE = 44100;

Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget),
    inputDevice(nullptr),
    inputContext(nullptr),
    currentBuffer(0),
    currentPlaybackSource(0)
{
    ui->setupUi(this);
    initializeAudio();
}

Widget::~Widget()
{
    delete ui;

    alDeleteSources(1, &currentPlaybackSource);
    alDeleteBuffers(1, &currentBuffer);

    alcMakeContextCurrent(nullptr);
    alcDestroyContext(inputContext);
    alcCloseDevice(inputDevice);
}


void Widget::initializeAudio()
{

    inputDevice = alcOpenDevice(nullptr);
    if (!inputDevice)
        throw("failed to get sound device");

    inputContext = alcCreateContext(inputDevice, nullptr);
    if(!inputContext)
        throw("Failed to set sound context");

    if (!alcMakeContextCurrent(inputContext))
        throw("failed to make context current");

    const ALCchar* name = nullptr;
    if (alcIsExtensionPresent(inputDevice, "ALC_ENUMERATE_ALL_EXT"))
        name = alcGetString(inputDevice, ALC_ALL_DEVICES_SPECIFIER);

    if (!name || alcGetError(inputDevice) != AL_NO_ERROR)
        name = alcGetString(inputDevice, ALC_DEVICE_SPECIFIER);

    qDebug() << "Opened " << name;

    connect(ui->captureBtn, &QPushButton::clicked, this, &Widget::on_captureBtn_clicked);
    connect(ui->stopBtn, &QPushButton::clicked, this, &Widget::on_stopBtn_clicked);
}

void Widget::on_stopBtn_clicked()
{

    ui->status_value->setText("Recording stopped");
    alcCaptureStop(inputDevice);
    alDeleteSources(1, &currentPlaybackSource);
    alDeleteBuffers(1, &currentBuffer);
}

void Widget::on_connectBtn_clicked()
{
    ui->status_value->setText("connected...");
}

void Widget::on_captureBtn_clicked()
{
    ui->status_value->setText("Recording...");

    //recording code
    alcCaptureStart(inputDevice);

    alGenBuffers(1, &currentBuffer);

    alGenSources(1, &currentPlaybackSource);

    QTimer* audioTimer = new QTimer(this);
    connect(audioTimer, &QTimer::timeout, this, &Widget::processAudio);
    audioTimer->start(5);

}

void Widget::processAudio()
{
    ALshort capturedData[CAP_SIZE] = {0};
    alcCaptureSamples(inputDevice, capturedData, CAP_SIZE);

    alBufferData(currentBuffer, AL_FORMAT_MONO16, capturedData, CAP_SIZE * sizeof(ALshort), 44100);

    alSourcei(currentPlaybackSource, AL_BUFFER, currentBuffer);
    alSourcePlay(currentPlaybackSource);

    ALint sState = 0;
    do {
        alGetSourcei(currentPlaybackSource, AL_SOURCE_STATE, &sState);
    } while (sState == AL_PLAYING);

}
