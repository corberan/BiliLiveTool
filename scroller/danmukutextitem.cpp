#include "danmukutextitem.h"

danmukuTextItem::danmukuTextItem(QString html, int stayTimeLongms)
{
    setViewState(readyToShow);
    this->setOpacity(0.7);
    this->setHtml(html);

    disappearAnimation = new QPropertyAnimation(this, "opacity");
    disappearAnimation->setDuration(600);
    disappearAnimation->setStartValue(1);
    disappearAnimation->setEndValue(0);

    appearAnimation = new QPropertyAnimation(this, "opacity");
    appearAnimation->setDuration(600);
    appearAnimation->setStartValue(this->opacity());
    appearAnimation->setEndValue(1);

    QTimer::singleShot(600, this, SLOT(startAppear()));
    QTimer::singleShot(stayTimeLongms, this, SLOT(startDisappear()));
    connect(disappearAnimation, SIGNAL(finished()), this, SLOT(setStateDestroy()));
}

void danmukuTextItem::setViewState(int state)
{
    itemViewState = state;
}

int danmukuTextItem::viewState()
{
    return itemViewState;
}

void danmukuTextItem::startDisappear()
{
    disappearAnimation->start();
}

void danmukuTextItem::startAppear()
{
    appearAnimation->start();
}

void danmukuTextItem::setStateDestroy()
{
    setViewState(tobeDestroy);
}
