#ifndef MYDOUYIN_H
#define MYDOUYIN_H

#include <QWidget>
#include"QObject"
#include <QCloseEvent>
#include <QVBoxLayout>

#include<qmap>
#include<QThread>

namespace Ui {
class MyDouYin;
}


class MyDouYin : public QWidget
{
    Q_OBJECT

public:
    explicit MyDouYin(QWidget *parent = 0);
    ~MyDouYin();

    enum PlayType
    {
        FromLocal,
        FromOnline
    };
signals:
    //void SIG_updateProcess(qint64,qint64);
    void SIG_nextVideo();
    void SIG_lastVideo();
    void SIG_openMyMessage();
    void SIG_openBroadCast();
    void SIG_startBroadCast();
    void SIG_stopBroadCast();
    void SIG_openRecordVideo();

private slots:
    void on_tb_town_clicked();

    void on_tb_attention_clicked();

    void on_tb_recommend_clicked();
    void on_pb_close_clicked();

    void on_pb_min_clicked();

    void on_pb_search_clicked();
    void on_pb_search_2_clicked();

    void on_tb_me_clicked();

    void on_tb_broadcast_clicked();

    void on_pushButton_clicked();

public:
    Ui::MyDouYin *ui;

    int m_userID;
    PlayType m_playType;
    QString filePath;
public slots:
    void on_tb_start_clicked();
    void on_tb_end_clicked();
};

#endif // MYDOUYIN_H
