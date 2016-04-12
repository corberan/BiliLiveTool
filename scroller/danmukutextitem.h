#ifndef DANMUKUTEXTITEM_H
#define DANMUKUTEXTITEM_H

#include <QObject>
#include <QGraphicsTextItem>
#include <QTimer>
#include <QPropertyAnimation>
#include <QString>

class danmukuTextItem : public QGraphicsTextItem
{
Q_OBJECT
private:
    int itemViewState;
    QPropertyAnimation *disappearAnimation;
    QPropertyAnimation *appearAnimation;

private slots:
    void startAppear();
    void startDisappear();
    void setStateDestroy();

public:
    enum VIEWSTATE{
        tobeDestroy = -1,
        readyToShow = 0,
        onShow = 1,
        needToRemove = 2
    };
    danmukuTextItem(QString html, int stayTimeLongms);
    void setViewState(int state);
    int viewState();

};

#endif // DANMUKUTEXTITEM_H
