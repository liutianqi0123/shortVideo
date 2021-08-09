#ifndef MYVIDEODIALOG_H
#define MYVIDEODIALOG_H

#include <QDialog>
#include<QMap>
#include"Packdef.h"
#include"logindialog.h"
//#include"customwidget.h"
namespace Ui {
class myVideoDialog;
}

class myVideoDialog : public QDialog
{
    Q_OBJECT
signals:
    //void SIG_UploadFile(QString filePath , QString imgPath , Hobby by );
    void SIG_UploadFiles(QString filePath , QString imgPath , Hobby by );
    void SIG_PlayUrl(QString);
    void SIG_AddFriend(int,QString,QString);
    void SIG_openAttentionList();
public:
    explicit myVideoDialog(QWidget *parent = 0);
    ~myVideoDialog();
    void clear();
    QString SaveVideoJif(QString FilePath);



public slots:
    bool eventFilter(QObject *watch, QEvent *event);
    void slot_send_UserName(QString name, QString userId);
private slots:
    void on_pb_upload_clicked();

    //void on_pb_upload_2_clicked();

    void on_pb_brower_clicked();
    void slot_updateProcess(qint64,qint64);
    void slot_setMovie(int,QString,QString);

    //void on_pb_getFriend_clicked();

    void on_pb_getFriend_clicked();

    void on_pb_attention_clicked();

private:
    Ui::myVideoDialog *ui;

    char m_userName[MAX_SIZE];
    int m_userId;
    QString m_filePath;
    QString m_imgPath;
    QMap<QString , QString> m_mapButtonNameToRtmpUrl;
};

#endif // MYVIDEODIALOG_H
