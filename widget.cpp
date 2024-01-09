// widget.cpp
#include "widget.h"
#include "ui_widget.h"
#include <QDebug>
#include <QtConcurrent/QtConcurrent>

Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget),
    inputDevice(nullptr),
    inputContext(nullptr),
    inputSource(0),
    outputSource(0),
    isCapturing(false)
{
    ui->setupUi(this);
    initializeAudio();
}

Widget::~Widget()
{
    delete ui;
    alDeleteSources(1, &inputSource);
    alcDestroyContext(inputContext);
    alcCaptureCloseDevice(inputDevice);
}

void Widget::initializeAudio()
{
    const ALCchar* deviceName;

    deviceName = alcGetString(nullptr, ALC_DEFAULT_DEVICE_SPECIFIER);

    inputDevice = alcOpenDevice(deviceName);

    if (!inputDevice)
    {
        qWarning() << "Failed to open audio capture device";
        return;
    }

    qDebug() << "device : " << inputDevice;

    inputContext = alcCreateContext(inputDevice, nullptr);



    alcMakeContextCurrent(inputContext);
    alGenSources(1, &inputSource);
    alGenSources(1, &outputSource);

    connect(ui->captureBtn, &QPushButton::clicked, this, &Widget::on_captureBtn_clicked);
    connect(ui->stopBtn, &QPushButton::clicked, this, &Widget::on_stopBtn_clicked);
}

void Widget::on_connectBtn_clicked()
{
    ui->status_value->setText("connected...");
}

void Widget::on_captureBtn_clicked()
{
    ui->status_value->setText("Recording...");

    alcMakeContextCurrent(inputContext);

    alcCaptureStart(inputDevice);
    alSourcePlay(inputSource);


    alcCaptureStart(inputDevice);
    alSourcePlay(inputSource);

    isCapturing = true;

    // QtConcurrent::run([this]() {
    //     while (isCapturing) {
    //         processAudio();
    //     }
    // });

    QTimer* audioTimer = new QTimer(this);
    connect(audioTimer, &QTimer::timeout, this, &Widget::processAudio);
    audioTimer->start(20);

}

void Widget::processAudio()
{
    ALint samplesAvailable;
    alcGetIntegerv(inputDevice, ALC_CAPTURE_SAMPLES, 1, &samplesAvailable);

    if (samplesAvailable > 0) {
        ALshort buffer[samplesAvailable];
        alcCaptureSamples(inputDevice, buffer, samplesAvailable);

        ALenum alError = alGetError();
        if (alError != AL_NO_ERROR) {
            qCritical() << "OpenAL Error: " << alGetString(alError);
            return;
        }

        alSourceQueueBuffers(outputSource, 1, &inputSource);

        alError = alGetError();
        if (alError != AL_NO_ERROR) {
            qCritical() << "OpenAL Error (Queue Buffers): " << alGetString(alError);
        }
    }
}

void Widget::on_stopBtn_clicked()
{
    isCapturing = false;
    ui->status_value->setText("Recording stopped");
    alcCaptureStop(inputDevice);
    alcMakeContextCurrent(nullptr);
    alSourceStop(inputSource);
}

