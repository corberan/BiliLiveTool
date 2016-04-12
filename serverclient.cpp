#include "serverclient.h"

ServerClient::ServerClient(QObject *parent) : QObject(parent)
{
    CID_INFO_URL = "http://live.bilibili.com/api/player?id=cid:";
    DEFAULT_COMMENT_HOST = "livecmt-1.bilibili.com";
    DEFAULT_COMMENT_PORT = 788;
    PROTOCOL_VERSION = 1;
//    SOCKET_RECV_BUFFER_LENGTH = 50 * 1024;

    manager = new QNetworkAccessManager(this);
    heartBeatTimer = new QTimer(this);
    socket = new QTcpSocket(this);
//    socket->setReadBufferSize(SOCKET_RECV_BUFFER_LENGTH);

    connect(socket, SIGNAL(connected()), this, SLOT(connected()));
    connect(socket, SIGNAL(disconnected()), this, SLOT(disconnected()));
//    connect(socket, SIGNAL(bytesWritten(qint64)), this, SLOT(bytesWritten(qint64)));
    connect(socket, SIGNAL(readyRead()), this, SLOT(readyRead()));
    connect(socket, SIGNAL(error(QAbstractSocket::SocketError)),
            this, SLOT(connectionError(QAbstractSocket::SocketError)));

    connect(manager, SIGNAL(finished(QNetworkReply*)),
            this, SLOT(replyFinished(QNetworkReply*)));

    connect(heartBeatTimer, SIGNAL(timeout()), this, SLOT(sendHeartBeatMsg()));

    qsrand(time(NULL));

    // 更新地址 http://blog.sina.com.cn/s/blog_1597ddcc40102we5j.html
    appVersionNum = 100; // 1.0.0

}

void ServerClient::doConnect(int roomID)
{
    postAction("正在获取房间信息...");
    this->roomID = roomID;
    QString url = "http://live.bilibili.com/" + QVariant(this->roomID).toString();
    manager->get(QNetworkRequest(QUrl(url)));
}

void ServerClient::doDisConn()
{
    if (heartBeatTimer->isActive())
    {
        heartBeatTimer->stop();
    }
    socket->close();
}

void ServerClient::connected()
{
    postAction("连接服务器正常");
    heartBeatTimer->setSingleShot(true);
    heartBeatTimer->start(1000);
}

void ServerClient::disconnected()
{
    postAction("已从服务器断开连接");
}

void ServerClient::connectionError(QAbstractSocket::SocketError error)
{
    QString errorStr;
    switch (error)
    {
    case QAbstractSocket::AddressInUseError:
        errorStr = "SOCKET ERROR: Address is already in use";
        break;
    case QAbstractSocket::ConnectionRefusedError:
        errorStr = "SOCKET ERROR: Connection refused";
        postAction(Intent::ACTION_RECONN, errorStr);
        break;
    case QAbstractSocket::HostNotFoundError:
        errorStr = "SOCKET ERROR: Host not found";
        break;
    case QAbstractSocket::RemoteHostClosedError:
        errorStr = "SOCKET ERROR: Remote host closed";
        postAction(Intent::ACTION_RECONN, errorStr);
        break;
    default:
        errorStr = "错误代码" + QString(error);
        break;
    }
    postAction(Intent::ACTION_LOG_ERROR, "Socket连接出错，错误信息：" + errorStr);
}

//void ServerClient::bytesWritten(qint64 bytes)
//{
//    qDebug() << bytes << " bytes written...";
//}

void ServerClient::readyRead()
{
    qint64 avaliableSize = socket->bytesAvailable();
//    qDebug() << "available: " << avaliableSize << "求余" << avaliableSize % 1440;
    if (avaliableSize < 16)
    {
        qDebug() << "收到错误的数据";
        return;
    }
    else if (avaliableSize % 1440 == 0)
    {
//        qDebug() << "mtu up" << avaliableSize;
        return;
    }

    QByteArray bytes = socket->readAll();
    emit dataReceived(bytes);
}

void ServerClient::replyFinished(QNetworkReply *reply)
{
    reply->ignoreSslErrors();
    int error = reply->error();
    if (error == QNetworkReply::NoError)
    {
        QString responseStr = QString(reply->readAll());
        if (responseStr.isEmpty()) return;
        QString url = reply->url().toString();

        // live.bilibili.com/roomurl
        if (url.indexOf("live.bilibili.com/" + QVariant(roomID).toString()) > -1)
        {
            postAction("房间标题：" + getStrBetween(responseStr, "<title>", "</title>").remove(" - 哔哩哔哩直播"));
            QString realRoomID = getStrBetween(responseStr, "var ROOMID = ", ";");
            if (realRoomID.toInt() > 0)
            {
                roomID = realRoomID.toInt();
                postAction("房间号映射为：" + realRoomID);
            }
            else
            {
                postAction("可能是bilibili的网页做了更新，无法获取到房间号，继续使用输入的房间号。");
                postAction("如果你不是很忙，希望可以反馈这个问题给我：liuz430524@hotmail.com，谢谢。");
            }
            QString apiUrl = "http://live.bilibili.com/api/player?id=cid:" + QVariant(roomID).toString();
            manager->get(QNetworkRequest(QUrl(apiUrl)));
        }
        // live.bilibili.com/api/player?id=cid:roomid
        else if (url.indexOf("live.bilibili.com/api/player?id=cid:" + QVariant(roomID).toString()) > -1)
        {
            postAction("房间状态：" + getStrBetween(responseStr, "<state>", "</state>"));
            QString server = getStrBetween(responseStr, "<server>", "</server>");
            if (!server.isEmpty())
            {
                DEFAULT_COMMENT_HOST = server;
            }

            postAction("正在连接...");
            socket->connectToHost(DEFAULT_COMMENT_HOST, DEFAULT_COMMENT_PORT);
            if(!socket->waitForConnected(5000))
            {
                postAction(Intent::ACTION_LOG_ERROR, socket->errorString());
            }
            else
            {
                sendJoinMsg(roomID);
            }
            // 更新
            QString url = "http://blog.sina.com.cn/s/blog_1597ddcc40102we5j.html";
            manager->get(QNetworkRequest(QUrl(url)));
        }
        // 更新信息
        else if (url.indexOf("blog.sina.com.cn") > -1)
        {
            QString version = getStrBetween(responseStr, "update_version", "update_version");
            int versionNum;
            if ((versionNum = QVariant(version.replace(" ", "")).toInt()) > 0)
            {
                if (versionNum > appVersionNum)
                {
                    QString instruction = getStrBetween(responseStr, "update_instruction", "update_instruction");
                    QString updateUrl = getStrBetween(responseStr, "update_url", "update_url");
                    postAction(Intent::ACTION_LOG_NOTICE, "弹幕姬有新版本：" + version);
                    postAction(Intent::ACTION_LOG_NOTICE, instruction);
                    postAction(Intent::ACTION_UPDATE, updateUrl);
                }
            }
        }
    }
    else
    {
        postAction(Intent::ACTION_LOG_ERROR, "错误代码：" + QVariant(error).toString() + " ，请检查房间号是否输入正确，或者网络是否通畅。");
        postAction("可以参考 http://doc.qt.io/qt-5/qnetworkreply.html#NetworkError-enum 中的错误代码对应信息。");
    }
    reply->deleteLater();
}

void ServerClient::sendSocketData(int totalLen, short headLen, short version, int action, int param5, char *data)
{
    QDataStream os(socket);
    os << totalLen << headLen << version << action << param5;
    if(data != NULL) socket->write(data);
}

void ServerClient::sendJoinMsg(int roomid)
{
    qint64 userid = 100000000 + 10000 * qrand();
    QByteArray body = QString("{\"roomid\": " + QVariant(roomid).toString() + ", \"uid\": " + QVariant(userid).toString() + "}").toLatin1();
    char *bytes = body.data();
    sendSocketData(16 + body.length(), 16, PROTOCOL_VERSION, 7, 1, bytes);
}

void ServerClient::sendHeartBeatMsg()
{
    if (heartBeatTimer->isSingleShot())
    {
        heartBeatTimer->setSingleShot(false);
        heartBeatTimer->setInterval(10000);
        if (heartBeatTimer->isActive())
        {
            heartBeatTimer->stop();
        }
        heartBeatTimer->start();
    }
    sendSocketData(16, 16, PROTOCOL_VERSION, 2, 1, NULL);
}

QString ServerClient::getStrBetween(QString sub, QString prev, QString after)
{
    QString ret = "";
    int prev_loc = sub.indexOf(prev);
    if (prev_loc > -1)
    {
        prev_loc += prev.length();
        int after_loc = sub.indexOf(after, prev_loc);
        if (after_loc > prev_loc)
        {
            ret = sub.mid(prev_loc, after_loc - prev_loc);
        }
    }
    return ret;
}

void ServerClient::postAction(QString content)
{
    emit actionPrepared(Intent(Intent::ACTION_LOG_INFO, content));
}

void ServerClient::postAction(int action, QString content)
{
    emit actionPrepared(Intent(action, content));
}
