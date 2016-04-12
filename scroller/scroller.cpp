#include "scroller.h"

Scroller::Scroller(QWidget *parent) : QWidget(parent)
{
    QRect screenRect = QDesktopWidget().availableGeometry(-1);
    screenWid = screenRect.width();
    screenHgt = screenRect.height();

    scrollerWid = 250;
    scrollerHgt = screenHgt;
    scrollerPosX = screenWid - scrollerWid;
    scrollerPosY = screenHgt - scrollerHgt;

    stayTimeLong = 4000;

    view = new QGraphicsView();
    scene = new QGraphicsScene();

    view->setGeometry(scrollerPosX, scrollerPosY, scrollerWid, scrollerHgt);
    view->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    view->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
//    view->setWindowOpacity(0);
    view->setWindowFlags(windowFlags() |Qt::SubWindow | Qt::WindowStaysOnTopHint | Qt::FramelessWindowHint);
    view->setStyleSheet("background: transparent; border: 0px;");
//    view->setBackgroundBrush(QBrush(QColor(0, 0, 0, 192)));
    view->setContentsMargins(0, 0, 0, 0);
    view->setAttribute(Qt::WA_TranslucentBackground);
    view->setAttribute(Qt::WA_TransparentForMouseEvents);

    scene->setSceneRect(0, 0, scrollerWid, scrollerHgt);
//    scene->setBackgroundBrush(QBrush(QColor(255, 0, 0, 96)));

    view->setScene(scene);

    danmukuItemGroup = new QGraphicsItemGroup();
    scene->addItem(danmukuItemGroup);

    animationTimeline = new QTimeLine(1000);
    animationTimeline->setLoopCount(1);
//    animationTimeline->setUpdateInterval(20);
    scrollAnimation = new QGraphicsItemAnimation();
    scrollAnimation->setTimeLine(animationTimeline);
}

Scroller::~Scroller()
{

}

void Scroller::showView()
{
    QPoint oldPos = view->pos();
    danmukuItemGroup->childItems().clear();
    danmukuItemGroup->setPos(0, view->height());
//    scene->addItem(danmukuItemGroup);
    view->show();
    scene->setSceneRect(view->rect());
    view->move(oldPos);
}

void Scroller::closeView()
{
    animationTimeline->stop();
//    scene->clear();
//    scene->update();
//    view->update();
    view->close();
}

void Scroller::showWindowFrame(bool flag)
{
    if (flag)
    {
        view->setWindowFlags(Qt::SubWindow | Qt::WindowStaysOnTopHint | windowFlags() &~ Qt::FramelessWindowHint);
        view->setStyleSheet("background: #ff0000;");
    }
    else
    {
        view->setWindowFlags(windowFlags() | Qt::SubWindow | Qt::WindowStaysOnTopHint | Qt::FramelessWindowHint);
        view->setStyleSheet("background: transparent; border: 0px;");
    }
    // showView是否调用取决于是否开启侧边栏
}

void Scroller::refreshScroller()
{
    if (scene->sceneRect().width() != view->rect().width() || scene->sceneRect().height() != view->rect().height())
    {
        scene->setSceneRect(view->rect());
//        qDebug() << "changed";
    }

    animationTimeline->stop();

    QList<QGraphicsItem *> allItems = danmukuItemGroup->childItems();
    foreach (QGraphicsItem *item, allItems) {
        danmukuTextItem *danmukuItem = (danmukuTextItem *)item;
        int viewState = danmukuItem->viewState();
        if(viewState == danmukuItem->tobeDestroy)
        {
            danmukuItemGroup->removeFromGroup(item);
        }
    }

    scrollAnimation->setItem(danmukuItemGroup);
    scrollAnimation->setPosAt(1, QPointF(0, view->height()-(danmukuItemGroup->boundingRect().y()+danmukuItemGroup->boundingRect().height())));
    animationTimeline->start();

}

void Scroller::setStayTimeLong(int mstime)
{
    if (time > 0)   stayTimeLong = mstime;
}

void Scroller::setScrollerFont(const QFont &font)
{
    scene->setFont(font);
}

void Scroller::setViewRect(const QRect &rect)
{
    if (rect.width() > 0 && rect.height() > 0)
    {
        QPoint leftTop = mapFromParent(rect.topLeft());
        view->move(leftTop);
        view->resize(rect.width(), rect.height());
    }
}

QRect Scroller::getViewRect()
{
    return QRect(mapToParent(view->pos()), QSize(view->rect().width(), view->rect().height()));
}

void Scroller::appendHtml(QString html)
{
    if (mutex.tryLock(1000))
    {
        danmukuTextItem *danmukuItem = new danmukuTextItem(html, stayTimeLong);
        danmukuItem->document()->setDocumentMargin(0);
        danmukuItem->setTextWidth(scene->width());
        danmukuItem->setFont(scene->font());
        danmukuItem->setPos(0, danmukuItemGroup->y() + danmukuItemGroup->boundingRect().y() + danmukuItemGroup->boundingRect().height());
        danmukuItemGroup->addToGroup(danmukuItem);

        refreshScroller();
        mutex.unlock();
    }
}
