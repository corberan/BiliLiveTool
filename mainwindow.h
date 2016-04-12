#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QDebug>
#include <QSettings>
#include <QMessageBox>
#include <QStandardItemModel>
#include <QList>
#include <QStandardPaths>
#include <QDir>
#include <QLibrary>
#include <QTimer>
#include <QSystemTrayIcon>
#include <QFontDialog>
#include <QFont>
#include <QColor>
#include <QColorDialog>
#include <QDesktopServices>
#include <QUrl>

#include "livedanmuhandler.h"
#include "plugin/intent.h"
#include "scroller/scroller.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
    LiveDanmuHandler *liveDanmuHandler; // 弹幕收取解析
    Scroller *sideScroller;
    QStandardItemModel tableModel;
    QList<BiliLiveDanmuToolPlugin *> pluginsList;
    QTimer *reConnTimer;
    QSystemTrayIcon *infoNotifier;

    int danmukuCount, giftCount;
    int selectedRowIndex;

    bool isNotify;

    void readSettings();
    void saveSettings();

    void loadPlugin(QString pluginPath);

    enum LOGTYPE{
        NORMAL,
        CUSTOM,
        DANMUKU,
        WELCOME,
        GIFT,
        NOTICE,
        ERROR
    };

    void appendLog(int logType, QString title, QString content);
    void popNotice(QString title, QString content);

    // 侧边栏属性
    QString scroller_fontFamily;
    QString scroller_fontStyle;
    int scroller_fontSize;

    QString scroller_unameColor;
    QString scroller_textColor;

signals:
    void messageGenerated(QString); // 通知侧边栏

protected:
    void closeEvent(QCloseEvent *event);

private slots:
    void notifyViewerChanged(int count);
    void handlerAction(Intent intent);
    void sendBroadcast(BiliLiveDanmuToolPlugin::ReceivedDanmukuProperties danmuku); // 转发弹幕给所有启动的插件
    void reConnect();
    void autoScrolltoEnd();

    // 插件
    void startPlugin();
    void stopPlugin();
    void managePlugin();
    void showPluginMessage(QString title, QString content);

    void selectedRow(QModelIndex);

    void on_pushButton_conn_clicked();
    void on_pushButton_disconn_clicked();
    void on_tabWidget_tabBarClicked(int index);
    void on_checkBox_keepTop_clicked(bool checked);
    void on_checkBox_noticeOn_clicked(bool checked);
    void on_pushButton_setFont_clicked();
    void on_pushButton_setColor_clicked();
    void on_spinBox_stayTime_valueChanged(int arg1);
    void on_checkBox_scollerOn_clicked(bool checked);
    void on_checkBox_scrollerResize_clicked(bool checked);
};

#endif // MAINWINDOW_H
