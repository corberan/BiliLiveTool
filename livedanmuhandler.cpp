#include "livedanmuhandler.h"

LiveDanmuHandler::LiveDanmuHandler(QObject *parent) : QObject(parent)
{
    serverClient = NULL;
}

void LiveDanmuHandler::connectServer(int roomID)
{
    serverClient = new ServerClient();
    connect(serverClient, SIGNAL(actionPrepared(Intent)), this, SLOT(dispatchAction(Intent)));
    connect(serverClient, SIGNAL(dataReceived(QByteArray)), this, SLOT(parseDataToDanmu(QByteArray)));
    serverClient->doConnect(roomID);
}

void LiveDanmuHandler::disConnServer()
{
    if (serverClient != NULL)
    {
        serverClient->doDisConn();
        delete serverClient;
        serverClient = NULL;
    }
}

void LiveDanmuHandler::dispatchAction(Intent intent)
{
    emit actionPrepared(intent);
}

void LiveDanmuHandler::parseDataToDanmu(QByteArray data)
{
    if (!data.isNull() && !data.isEmpty())
    {
        QDataStream dataStream(&data, QIODevice::ReadOnly);
        qint64 dataLength = data.length();
        if (dataLength < 16)
        {
            emit actionPrepared(Intent(Intent::ACTION_LOG_ERROR,"接收到的数据异常"));
        }
        else if (dataLength > 16)
        {
            int givenLength;
            dataStream >> givenLength;
            if (dataLength == givenLength)
            {
                short param1, param2;
                int action, param4;
                dataStream >> param1 >> param2 >> action >> param4;
                action -= 1;
//                qDebug() << action;
                if (action == 2)
                {
                    int userCount;
                    dataStream >> userCount;
//                    qDebug() << userCount;
                    emit viewerCountChanged(userCount);
                }
                else if (action == 4)
                {
                    uint len = givenLength - 16;
                    char *msgJsonBytes = new char[len + 1]();
                    dataStream.readRawData(msgJsonBytes, len);
                    QString msgJsonStr = QString::fromUtf8(msgJsonBytes);
                    delete []msgJsonBytes;
//                    qDebug() << len;
//                    qDebug() << "json: " << msgJsonStr;
                    QJsonDocument msgDoc = QJsonDocument::fromJson(msgJsonStr.toUtf8());
                    if (!msgDoc.isNull())
                    {
                        QJsonObject msgObj = msgDoc.object();
                        QString msgType = msgObj["cmd"].toString();

                        // 弹幕封装
                        BiliLiveDanmuToolPlugin::ReceivedDanmukuProperties danmukuPros;
                        danmukuPros.rawData = msgJsonStr.toUtf8();
                        danmukuPros.danmukuTypeStr = msgType;

                        if (msgType == "DANMU_MSG")
                        {
//                            int uid = msgObj["info"][2][0];
                            QJsonArray info = msgObj["info"].toArray();
                            QString uname = info[2].toArray()[1].toString();
                            QString danmuku = info[1].toString();

                            danmukuPros.danmukuType = danmukuPros.DANMUKU;
                            danmukuPros.userName = uname;
                            danmukuPros.danmukuText = danmuku;

                        }
                        else if(msgType == "SEND_GIFT")
                        {
                            QJsonObject data = msgObj["data"].toObject();
                            QString giftName = data["giftName"].toString();
                            QString uname = data["uname"].toString();
                            int giftNum = data["num"].toInt();

                            danmukuPros.danmukuType = danmukuPros.GIFT;
                            danmukuPros.userName = uname;
                            danmukuPros.giftName = giftName;
                            danmukuPros.giftNum = giftNum;

                        }
                        else if(msgType == "WELCOME")
                        {
                            QJsonObject data = msgObj["data"].toObject();
                            QString uname = data["uname"].toString();

                            danmukuPros.danmukuType = danmukuPros.WELCOME;
                            danmukuPros.userName = uname;
                        }
                        else
                        {
                            danmukuPros.danmukuType = -999;
                        }

                        emit messageReceived(danmukuPros);
                    }
                    else
                    {
                        qDebug() << "无法生成正确的json";
                    }
                }
            }
            else if (16 < givenLength && givenLength < dataLength)
            {
//                qDebug() << "多条信息" << givenLength << dataLength;
                QByteArray firstData = data.mid(0, givenLength);
                QByteArray remainData = data.mid(givenLength);
                parseDataToDanmu(firstData);
                parseDataToDanmu(remainData);
            }
        }
        else
        {
            int givenLength;
            dataStream >> givenLength;
            if (givenLength == 16)
            {
                emit actionPrepared(Intent(Intent::ACTION_LOG_INFO,"服务器认证成功"));
            }
        }
    }
}
