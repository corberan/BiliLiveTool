#ifndef INTENT_H
#define INTENT_H

#include <QString>

class Intent
{
public:
    enum actions{
        ACTION_ERROR_ALTER,
        ACTION_RECONN,
        ACTION_UPDATE,
        ACTION_LOG_INFO,
        ACTION_LOG_ERROR,
        ACTION_LOG_NOTICE,
        ACTION_LOG_CUSTOM
    };

    Intent();
    Intent(int action, QString content);

    int getAction();
    QString getContent();

private:
    int action;
    QString content;
};

#endif // INTENT_H
