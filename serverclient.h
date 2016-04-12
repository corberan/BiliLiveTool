#ifndef SERVERCLIENT_H
#define SERVERCLIENT_H

#include <QObject>
#include <QTcpSocket>
#include <QAbstractSocket>
#include <QNetworkAccessManager>
#include <QUrl>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QTime>
#include <QJsonObject>
#include <QJsonDocument>
#include <QTimer>
#include <QDataStream>

#include "plugin/intent.h"

class ServerClient : public QObject
{
    Q_OBJECT
public:
    explicit ServerClient(QObject *parent = 0);
    void doConnect(int roomID);
    void doDisConn();

signals:
    void actionPrepared(Intent); // 由danmuhandler转发给MainWindow处理
    void dataReceived(QByteArray); // 发给danmuhandler处理

private slots:
    void connected();
    void disconnected();
    void connectionError(QAbstractSocket::SocketError);
//    void bytesWritten(qint64 bytes);
    void readyRead(); 
    void replyFinished(QNetworkReply* reply);
    void sendHeartBeatMsg();

private:
    QTcpSocket *socket;
    QString CID_INFO_URL;
    QString DEFAULT_COMMENT_HOST;
    int DEFAULT_COMMENT_PORT;
    int PROTOCOL_VERSION;
//    qint64 SOCKET_RECV_BUFFER_LENGTH;
    QNetworkAccessManager *manager;
    int roomID;
    QTimer *heartBeatTimer;
    // functions
    void sendSocketData(int totalLen, short headLen, short version, int action, int param5, char *data);
    void sendJoinMsg(int roomID);
    // util
    QString getStrBetween(QString sub, QString prev, QString after);

    void postAction(QString content);
    void postAction(int action, QString content);

    int appVersionNum;
};

#endif // SERVERCLIENT_H
