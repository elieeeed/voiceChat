// widget.cpp
#include "widget.h"
#include "ui_widget.h"
#include <QDebug>
#include <QtConcurrent/QtConcurrent>

Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
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
    inputDevice = alcCaptureOpenDevice(nullptr, 44100, AL_FORMAT_MONO16, 4096);

    if (!inputDevice)
    {
        qWarning() << "Failed to open audio capture device";
        return;
    }

    inputContext = alcCreateContext(inputDevice, nullptr);

    alcMakeContextCurrent(inputContext);
    alGenSources(1, &inputSource);

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
    alcCaptureStart(inputDevice);
    alSourcePlay(inputSource);


    alcCaptureStart(inputDevice);
    alSourcePlay(inputSource);

    isCapturing = true;

    QtConcurrent::run([this]() {
        while (isCapturing) {
            processAudio();
        }
    });

}

void Widget::processAudio()
{
    ALint samplesAvailable;
    alcGetIntegerv(inputDevice, ALC_CAPTURE_SAMPLES, 1, &samplesAvailable);

    if (samplesAvailable > 0) {
        ALshort buffer[samplesAvailable];
        alcCaptureSamples(inputDevice, buffer, samplesAvailable);

        alSourceQueueBuffers(outputSource, 1, &inputSource);
    }
}

void Widget::on_stopBtn_clicked()
{
    isCapturing = false;
    ui->status_value->setText("Recording stopped");
    alcCaptureStop(inputDevice);
    alSourceStop(inputSource);
}

