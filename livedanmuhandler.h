#ifndef LIVEDANMUHANDLER_H
#define LIVEDANMUHANDLER_H

#include <QObject>
#include <QString>
#include <QByteArray>
#include <QDataStream>
#include <QIODevice>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDateTime>

#include "plugin/bililivedanmutoolplugin.h"
#include "serverclient.h"
#include "plugin/intent.h"

class LiveDanmuHandler : public QObject
{
    Q_OBJECT
public:
    explicit LiveDanmuHandler(QObject *parent = 0);
    void connectServer(int roomID);
    void disConnServer();

signals:
    void actionPrepared(Intent);
    void viewerCountChanged(int);
    void messageReceived(BiliLiveDanmuToolPlugin::ReceivedDanmukuProperties);

public slots:
    void dispatchAction(Intent intent);
    void parseDataToDanmu(QByteArray data);

private:
    ServerClient *serverClient;
};

#endif // LIVEDANMUHANDLER_H
