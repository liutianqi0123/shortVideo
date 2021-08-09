#ifndef BROADCAST_H
#define BROADCAST_H

#include <QDialog>
#include"videoitem.h"
#include<QTimer>
#include<QTime>
#include"screenrecorder.h"

#define DEFAULT_FRAME_RATE (15)

namespace Ui {
class broadcast;
}

class broadcast : public QDialog
{
    Q_OBJECT
signals:
    //void SIG_closeWebCam();
    //void SIG_openAudio();
private slots:
    //void on_pb_close_clicked();

    //void slot_sendAudioFrame(QByteArray ba);
    void on_tb_open_clicked();

    void on_tb_close_clicked();

    void on_tb_people_clicked();

    void on_pb_close_clicked();

public:
    explicit broadcast(QWidget *parent = 0);
    ~broadcast();

    Ui::broadcast *getUi() const;

    QString m_savePath;
    QString m_Video;
    QString m_Audio;

    ScreenRecorder *m_screenRecorder;

public slots:
    //bool eventFilter(QObject *watch, QEvent *event);
    void slot_equipInit(QString audioName, QString videoName);
private:
    Ui::broadcast *ui;
};

#endif // BROADCAST_H
