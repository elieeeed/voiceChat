// widget.cpp
#include "widget.h"
#include "ui_widget.h"
#include <QDebug>
#include <QtConcurrent/QtConcurrent>

const int CAP_SIZE = 44100;

Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget),
    inputDevice(nullptr),
    inputContext(nullptr)
{
    ui->setupUi(this);
    initializeAudio();
}

Widget::~Widget()
{
    delete ui;

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

// QtConcurrent::run([this]() {
//     while (isCapturing) {
//         processAudio();
//     }
// });

void Widget::on_captureBtn_clicked()
{
    ui->status_value->setText("Recording...");

    //recording code
    alcCaptureStart(inputDevice);

    ALuint buffer;
    alGenBuffers(1, &buffer);

    ALuint playbackSource;
    alGenSources(1, &playbackSource);

    currentBuffer = buffer;
    currentPlaybackSource = playbackSource;

    QTimer* audioTimer = new QTimer(this);
    connect(audioTimer, &QTimer::timeout, this, &Widget::processAudio);
    audioTimer->start(10);

}

void Widget::processAudio()
{
    ALshort capturedData[CAP_SIZE];
    alcCaptureSamples(inputDevice, capturedData, CAP_SIZE);

    alBufferData(currentBuffer, AL_FORMAT_MONO16, capturedData, CAP_SIZE * sizeof(ALshort), 44100);

    alSourceStop(currentPlaybackSource);

    alSourcei(currentPlaybackSource, AL_BUFFER, currentBuffer);
    alSourcePlay(currentPlaybackSource);

    ALint sState = 0;
    do {
        alGetSourcei(currentPlaybackSource, AL_SOURCE_STATE, &sState);
    } while (sState == AL_PLAYING);

}

