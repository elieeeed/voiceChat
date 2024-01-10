// widget.cpp
#include "widget.h"
#include "ui_widget.h"
#include <QDebug>
#include <QHostInfo>
#include <QNetworkInterface>

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
    alcCaptureCloseDevice(captureDevice);
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

    const ALCchar* playbackDeviceName = nullptr;
    if (alcIsExtensionPresent(inputDevice, "ALC_ENUMERATE_ALL_EXT"))
        playbackDeviceName = alcGetString(inputDevice, ALC_ALL_DEVICES_SPECIFIER);

    if (!playbackDeviceName || alcGetError(inputDevice) != AL_NO_ERROR)
        playbackDeviceName = alcGetString(inputDevice, ALC_DEVICE_SPECIFIER);

    qDebug() << "Playback device: " << playbackDeviceName;

    // Open capture device
    captureDevice = alcCaptureOpenDevice(nullptr, 44100, AL_FORMAT_MONO16, CAP_SIZE);
    if (!captureDevice)
        throw("failed to open capture device");

    const ALCchar* captureDeviceName = alcGetString(captureDevice, ALC_CAPTURE_DEVICE_SPECIFIER);
    qDebug() << "Capture device: " << captureDeviceName;

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
    QString ipAddress = getLocalIpAddress();

    QHostAddress hostAddress(ipAddress);
    if (!hostAddress.isNull() && hostAddress.protocol() == QAbstractSocket::IPv4Protocol) {
        qDebug() << "Connecting to IP: " << ipAddress;

        ui->status_value->setText("Connected to " + ipAddress);
    }
    else {

        ui->status_value->setText("Invalid IP address");
    }
}

void Widget::on_captureBtn_clicked()
{
    ui->status_value->setText("Recording...");

    captureDevice = alcCaptureOpenDevice(nullptr, 44100, AL_FORMAT_MONO16, CAP_SIZE);

    //recording code
    alcCaptureStart(captureDevice);

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

QString Widget::getLocalIpAddress() {
    QList<QHostAddress> ipAddressesList = QNetworkInterface::allAddresses();
    for (const QHostAddress& address : ipAddressesList)
    {
        if (address.protocol() == QAbstractSocket::IPv4Protocol && !address.isLoopback())
        {
            return address.toString();
        }
    }
    return "";
}
