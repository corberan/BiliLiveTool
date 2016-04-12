#ifndef SCROLLER_H
#define SCROLLER_H

#include <QWidget>
#include <QDesktopWidget>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsTextItem>
#include <QTextDocument>
#include <QDebug>
//#include <vector>
#include <QTimeLine>
#include <QGraphicsItemAnimation>
#include <QTime>
#include <QTimer>
#include <QMutex>
#include <QList>
#include <QGraphicsItemGroup>
#include "danmukutextitem.h"

namespace Ui {
class Scroller;
}

class Scroller : public QWidget
{
    Q_OBJECT

public:
    explicit Scroller(QWidget *parent = 0);
    ~Scroller();
    void showView();
    void closeView();
    void setStayTimeLong(int mstime);
    void setScrollerFont(const QFont &font);
    void showWindowFrame(bool flag);
    void setViewRect(const QRect &rect);
    QRect getViewRect();

private:
    Ui::Scroller *ui;

    uint screenWid;
    uint screenHgt;
    uint scrollerPosX;
    uint scrollerPosY;
    uint scrollerWid;
    uint scrollerHgt;

    int stayTimeLong;

    QGraphicsView *view;
    QGraphicsScene *scene;

    QGraphicsItemGroup *danmukuItemGroup;

    QGraphicsItemAnimation *scrollAnimation;
    QTimeLine *animationTimeline;

    QMutex mutex;

    void refreshScroller();

private slots:
    void appendHtml(QString);

};

#endif // SCROLLER_H
