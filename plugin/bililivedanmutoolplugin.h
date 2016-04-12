#ifndef BILILIVEDANMUTOOLPLUGIN_H
#define BILILIVEDANMUTOOLPLUGIN_H

#include "bililivedanmutoolplugin_global.h"

#include <QObject>
#include <QString>

class BILILIVEDANMUTOOLPLUGINSHARED_EXPORT BiliLiveDanmuToolPlugin : public QObject
{
    Q_OBJECT
public:
    explicit BiliLiveDanmuToolPlugin(QObject *parent = 0);

    // 弹幕类
    class ReceivedDanmukuProperties
    {
    public:
        enum danmukuTypeEnum{
            DANMUKU,
            WELCOME,
            GIFT
        };

        int danmukuType;
        QString danmukuTypeStr;
        QString userName;
        QString danmukuText;
        QString giftName;
        int giftNum;
        QString rawData;
    };

    // 开发者信息
    QString pluginName;
    QString pluginIntroduction;
    QString pluginVersion;
    QString pluginDeveloper;
    QString developerContactInfo;

    // 状态
    enum status{
        STOPPED,
        RUNNING
    };

    virtual void Start();
    virtual void Stop();
    virtual void Setting();

    virtual int state();

private:
    int pluginState;

signals:
    void message(QString, QString); // title, content 将显示在log和侧边栏内

public slots:
    void ReceivedDanmuku(BiliLiveDanmuToolPlugin::ReceivedDanmukuProperties danmuku);
    void dispatchMessage(QString title, QString content);

};

#ifdef __cplusplus
extern "C" {
#endif

BILILIVEDANMUTOOLPLUGINSHARED_EXPORT BiliLiveDanmuToolPlugin *createPlugin();

#ifdef __cplusplus
}
#endif

#endif // BILILIVEDANMUTOOLPLUGIN_H
