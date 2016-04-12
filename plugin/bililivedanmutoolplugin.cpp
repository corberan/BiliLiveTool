#include "bililivedanmutoolplugin.h"


BiliLiveDanmuToolPlugin::BiliLiveDanmuToolPlugin(QObject *parent) : QObject(parent)
{
    // init
    pluginState = STOPPED;
}

void BiliLiveDanmuToolPlugin::Start()
{
    // start
    pluginState = RUNNING;
}

void BiliLiveDanmuToolPlugin::Stop()
{
    // stop
    pluginState = STOPPED;
}

void BiliLiveDanmuToolPlugin::Setting()
{
    // set
}

int BiliLiveDanmuToolPlugin::state()
{
    return pluginState;
}

void BiliLiveDanmuToolPlugin::ReceivedDanmuku(BiliLiveDanmuToolPlugin::ReceivedDanmukuProperties danmuku)
{
    // do something
}

void BiliLiveDanmuToolPlugin::dispatchMessage(QString title, QString content)
{
    //
}

BiliLiveDanmuToolPlugin *createPlugin()
{
    BiliLiveDanmuToolPlugin *plugin = new BiliLiveDanmuToolPlugin();
    return plugin;
}
