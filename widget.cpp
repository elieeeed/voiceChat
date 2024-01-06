// widget.cpp
#include "widget.h"
#include "ui_widget.h"
#include <QDebug>

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
}

void Widget::stopRecording()
{
    alcCaptureStop(audioCaptureDevice);
    alcCaptureCloseDevice(audioCaptureDevice);
    ui->status_value->setText("Recording stopped");
}

void Widget::initializeAudio()
{

    QBuffer* circularBufferDevice = new QBuffer();
    circularBufferDevice->open(QIODevice::ReadWrite);

    destinationFile.setFileName("/tmp/test.raw");
    destinationFile.open(QIODevice::WriteOnly | QIODevice::Truncate);

    ALCdevice* device = alcCaptureOpenDevice(nullptr, 44100, AL_FORMAT_MONO16, 4096);

    if (!device)
    {
        qWarning() << "Failed to open audio capture device";
        return;
    }

    audioCaptureDevice = device;
    circularBuffer = circularBufferDevice;

    ALCcontext* context = alcCreateContext(audioCaptureDevice, nullptr);
    if (!context) {
        qWarning() << "Failed to create audio context";
        alcCloseDevice(audioCaptureDevice);
        return;
    }


    alcCaptureStart(audioCaptureDevice);
    alBufferData(circularBufferSource, AL_FORMAT_MONO16, nullptr, 44100 * 10, 44100);
    alSourceQueueBuffers(circularBufferSource, 1, &circularBufferSource);
    alSourcePlay(circularBufferSource);

    connect(ui->captureBtn, &QPushButton::clicked, this, &Widget::on_captureBtn_clicked);
}

void Widget::on_connectBtn_clicked()
{
    ui->status_value->setText("Recording...");
}

void Widget::on_captureBtn_clicked()
{
    ui->status_value->setText("Recording...");
    alcCaptureStart(audioCaptureDevice);

    QTimer::singleShot(5000, this, &Widget::stopRecording);

    QTimer::singleShot(100, this, &Widget::updateCircularBuffer);

    QByteArray circularBufferData = circularBuffer->buffer();

    ALuint circularBufferBuffer;
    alGenBuffers(1, &circularBufferBuffer);
    alBufferData(circularBufferBuffer, AL_FORMAT_MONO16, circularBufferData.constData(), circularBufferData.size(), 44100);

    alGenSources(1, &circularBufferSource);
    alSourcei(circularBufferSource, AL_BUFFER, circularBufferBuffer);
    alSourcei(circularBufferSource, AL_LOOPING, AL_TRUE);
    alSourcePlay(circularBufferSource);

}

void Widget::updateCircularBuffer()
{
    ALint samplesAvailable;
    alcGetIntegerv(audioCaptureDevice, ALC_CAPTURE_SAMPLES, 1, &samplesAvailable);

    if (samplesAvailable > 0)
    {
        ALshort audioBuffer[4096];
        alcCaptureSamples(audioCaptureDevice, audioBuffer, samplesAvailable);
        circularBuffer->write(reinterpret_cast<const char*>(audioBuffer), samplesAvailable * sizeof(ALshort));
    }

    QTimer::singleShot(100, this, &Widget::updateCircularBuffer);
}
