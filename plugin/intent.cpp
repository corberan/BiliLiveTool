#include "intent.h"

Intent::Intent()
{

}

Intent::Intent(int action, QString content)
{
    this->action = action;
    this->content = content;
}

int Intent::getAction()
{
    return action;
}

QString Intent::getContent()
{
    return content;
}
