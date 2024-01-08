// widget.h
#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QPushButton>
#include <QFile>
#include <QTimer>
#include <AL/al.h>
#include <AL/alc.h>
#include <QBuffer>


QT_BEGIN_NAMESPACE
namespace Ui { class Widget; }
QT_END_NAMESPACE

class Widget : public QWidget
{
    Q_OBJECT

public:
    Widget(QWidget *parent = nullptr);
    ~Widget();

private slots:
    void on_connectBtn_clicked();
    void on_captureBtn_clicked();
    void on_stopBtn_clicked();
    void processAudio();


private:
    Ui::Widget *ui;
    ALCcontext* inputContext;
    ALCdevice* inputDevice;
    ALuint inputSource;
    ALuint outputSource;

    bool isCapturing;

    void initializeAudio();
};

#endif // WIDGET_H
