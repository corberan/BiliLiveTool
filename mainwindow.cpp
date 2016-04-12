#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // 初始化
    reConnTimer = new QTimer(this);
    infoNotifier = new QSystemTrayIcon(this);
    liveDanmuHandler = new LiveDanmuHandler(this);
    sideScroller = new Scroller(this);

    connect(ui->plainTextEdit_logs, SIGNAL(textChanged()), this, SLOT(autoScrolltoEnd()));
    connect(reConnTimer, SIGNAL(timeout()), this, SLOT(reConnect()));
    connect(liveDanmuHandler,
            SIGNAL(actionPrepared(Intent)),
            this,
            SLOT(handlerAction(Intent)));
    connect(liveDanmuHandler,
            SIGNAL(messageReceived(BiliLiveDanmuToolPlugin::ReceivedDanmukuProperties)),
            this,
            SLOT(sendBroadcast(BiliLiveDanmuToolPlugin::ReceivedDanmukuProperties)));
    connect(liveDanmuHandler,
            SIGNAL(viewerCountChanged(int)),
            this,
            SLOT(notifyViewerChanged(int)));
    connect(this, SIGNAL(messageGenerated(QString)), sideScroller, SLOT(appendHtml(QString)));


    // 插件列表初始化
    tableModel.setColumnCount(5);
    tableModel.setHeaderData(0, Qt::Horizontal, "插件名");
    tableModel.setHeaderData(1, Qt::Horizontal, "介绍");
    tableModel.setHeaderData(2, Qt::Horizontal, "版本号");
    tableModel.setHeaderData(3, Qt::Horizontal, "开发者");
    tableModel.setHeaderData(4, Qt::Horizontal, "状态");

    ui->tableView_plugins->setModel(&tableModel);
    ui->tableView_plugins->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableView_plugins->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->tableView_plugins->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->tableView_plugins->setContextMenuPolicy(Qt::ActionsContextMenu);
    ui->tableView_plugins->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    
    // 右击表格菜单
    QAction *pluginStartAction = new QAction("启动", this);
    QAction *pluginStopAction = new QAction("停止", this);
    QAction *pluginManageAction = new QAction("管理", this);

    ui->tableView_plugins->addAction(pluginStartAction);
    ui->tableView_plugins->addAction(pluginStopAction);
    ui->tableView_plugins->addAction(pluginManageAction);

    connect(pluginStartAction, SIGNAL(triggered(bool)), this, SLOT(startPlugin()));
    connect(pluginStopAction, SIGNAL(triggered(bool)), this, SLOT(stopPlugin()));
    connect(pluginManageAction, SIGNAL(triggered(bool)), this, SLOT(managePlugin()));
    connect(ui->tableView_plugins, SIGNAL(pressed(QModelIndex)), this, SLOT(selectedRow(QModelIndex)));

    // 读取设置
    readSettings();

    // 设置
    if (ui->checkBox_keepTop->isChecked()) setWindowFlags(windowFlags() | Qt::WindowStaysOnTopHint);
    if (ui->checkBox_noticeOn->isChecked())
        isNotify = true;
    else
        isNotify = false;

    // 侧边栏停留时长
    sideScroller->setStayTimeLong(ui->spinBox_stayTime->value()*1000);
    // 侧边栏字体
    QFont fontUsing(scroller_fontFamily, scroller_fontSize);
    fontUsing.setStyleName(scroller_fontStyle);
    sideScroller->setScrollerFont(fontUsing);
    // 显示侧边栏
    if (ui->checkBox_scollerOn->isChecked())
    {
        emit on_checkBox_scrollerResize_clicked(ui->checkBox_scrollerResize->isChecked());
    }
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    QMessageBox::StandardButton button;
    button = QMessageBox::question(this, "退出程序",
        "你确定要退出吗?",
        QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
    if (button == QMessageBox::No) {
        event->ignore();
    }
    else if (button == QMessageBox::Yes) {
        // 保存设置
        saveSettings();
        emit ui->pushButton_disconn->clicked();
        // 关闭所有插件
        for(int i=0;i<pluginsList.size();++i)
        {
            if (pluginsList.at(i)->state() == BiliLiveDanmuToolPlugin::RUNNING) pluginsList.at(i)->Stop();
        }
        event->accept();
    }
}

void MainWindow::readSettings()
{
    QSettings settings(QSettings::IniFormat, QSettings::UserScope, "Bilibili", "liveDanmuTool");
    ui->lineEdit_roomID->setText(settings.value("MainWindow/lineEdit_roomID", "").toString());
    ui->checkBox_keepTop->setChecked(settings.value("MainWindow/checkBox_keepTop", true).toBool());
    ui->checkBox_noticeOn->setChecked(settings.value("MainWindow/checkBox_noticeOn", true).toBool());
    ui->checkBox_scollerOn->setChecked(settings.value("MainWindow/checkBox_scollerOn", true).toBool());
    ui->checkBox_scrollerResize->setChecked(settings.value("Scroller/checkBox_scrollerResize", false).toBool());
    ui->spinBox_stayTime->setValue(settings.value("Scroller/spinBox_stayTime", 4).toInt());
    scroller_fontFamily = settings.value("Scroller/scroller_fontFamily", "Heiti SC").toString();
    scroller_fontStyle = settings.value("Scroller/scroller_fontStyle", "Regular").toString();
    scroller_fontSize = settings.value("Scroller/scroller_fontSize", 14).toInt();
    scroller_unameColor = settings.value("Scroller/scroller_unameColor", "#4fc1e9").toString();
    scroller_textColor = settings.value("Scroller/scroller_textColor", "#ffffff").toString();
    sideScroller->setViewRect(settings.value("Scroller/scroller_geometry", QRect(0, 0, 0, 0)).toRect());
}

void MainWindow::saveSettings()
{
    QSettings settings(QSettings::IniFormat, QSettings::UserScope, "Bilibili", "liveDanmuTool");
    settings.beginGroup("MainWindow");
    settings.setValue("lineEdit_roomID", ui->lineEdit_roomID->text());
    settings.setValue("checkBox_autoReconn", ui->checkBox_autoReconn->isChecked());
    settings.setValue("checkBox_keepTop", ui->checkBox_keepTop->isChecked());
    settings.setValue("checkBox_noticeOn", ui->checkBox_noticeOn->isChecked());
    settings.setValue("checkBox_scollerOn", ui->checkBox_scollerOn->isChecked());
    settings.endGroup();
    settings.beginGroup("Scroller");
    settings.setValue("checkBox_scrollerResize", ui->checkBox_scrollerResize->isChecked());
    settings.setValue("spinBox_stayTime", ui->spinBox_stayTime->value());
    settings.setValue("scroller_fontFamily", scroller_fontFamily);
    settings.setValue("scroller_fontStyle", scroller_fontStyle);
    settings.setValue("scroller_fontSize", scroller_fontSize);
    settings.setValue("scroller_unameColor", scroller_unameColor);
    settings.setValue("scroller_textColor", scroller_textColor);
    settings.setValue("scroller_geometry", sideScroller->getViewRect());
    settings.endGroup();
}

void MainWindow::on_pushButton_conn_clicked()
{
    int roomID = 0;
    if ((roomID = ui->lineEdit_roomID->text().toInt()) <= 0)
    {
        QMessageBox::warning(this, "警告", "请输入正确的房间号", QMessageBox::Yes, QMessageBox::Yes);
        return;
    }
    ui->plainTextEdit_logs->clear();
    danmukuCount = giftCount = 0;
    // disable
    ui->lineEdit_roomID->setEnabled(false);
    ui->pushButton_conn->setEnabled(false);
    // 显示侧边栏
    sideScroller->showView();
    // 连接
    liveDanmuHandler->connectServer(roomID);
}

void MainWindow::on_pushButton_disconn_clicked()
{
    // 清除信息
    this->setWindowTitle("Bilibili直播弹幕姬");
    ui->label_statistic->setText("暂无统计信息");
    // 断开服务器
    liveDanmuHandler->disConnServer();
    // 关闭侧边栏
    sideScroller->closeView();
    // 重置为可连接状态
    ui->lineEdit_roomID->setEnabled(true);
    ui->pushButton_conn->setEnabled(true);
}

void MainWindow::reConnect()
{
    emit ui->pushButton_disconn->clicked();
    emit ui->pushButton_conn->clicked();
}

void MainWindow::on_tabWidget_tabBarClicked(int index)
{
    if (index == 1 && pluginsList.size() == 0)
    {
        QStringList pathList = QStandardPaths::standardLocations(QStandardPaths::DocumentsLocation);
        if (pathList.size() > 0)
        {
            QString documentPath = pathList[0];
            if (!documentPath.isNull() && !documentPath.isEmpty())
            {
                QString pluginsPath = documentPath + "/弹幕姬插件/";
                QDir pluginsDir(pluginsPath);
                if (pluginsDir.exists())
                {
                    pluginsDir.setFilter(QDir::Files);
                    QFileInfoList fileList = pluginsDir.entryInfoList();
                    for (int i=0;i<fileList.size();++i)
                    {
                        QFileInfo fileInfo = fileList.at(i);
                        if (fileInfo.suffix() == "dylib")
                        {
                            loadPlugin(fileInfo.absoluteFilePath());
//                            qDebug() << "载入了一个插件";
                        }
                    }
                    // 清除列表
                    tableModel.removeRows(0, tableModel.rowCount());
                    // 显示到列表
                    for(int i=0;i<pluginsList.size();++i)
                    {
                        BiliLiveDanmuToolPlugin *pluginClass = pluginsList.at(i);
                        tableModel.setItem(i, 0, new QStandardItem(pluginClass->pluginName));
                        tableModel.setItem(i, 1, new QStandardItem(pluginClass->pluginIntroduction));
                        tableModel.setItem(i, 2, new QStandardItem(pluginClass->pluginVersion));
                        tableModel.setItem(i, 3, new QStandardItem(pluginClass->pluginDeveloper));
                        if (pluginClass->state() == BiliLiveDanmuToolPlugin::STOPPED)
                            tableModel.setItem(i, 4, new QStandardItem("未启动"));
                        else if (pluginClass->state() == BiliLiveDanmuToolPlugin::RUNNING)
                            tableModel.setItem(i, 4, new QStandardItem("已启动"));
                        // connect
                        connect(pluginClass, SIGNAL(message(QString,QString)), this, SLOT(showPluginMessage(QString, QString)));
                    }
//                    QList<QStandardItem *> items;
//                    items.append(new QStandardItem(pluginClass->pluginName));
//                    items.append(new QStandardItem(pluginClass->pluginIntroduction));
//                    items.append(new QStandardItem(pluginClass->pluginVersion));
//                    items.append(new QStandardItem(pluginClass->pluginDeveloper));
//                    items.append(new QStandardItem("未启动"));
//                    tableModel.appendRow(items);

                    ui->tableView_plugins->resizeColumnsToContents();
                }
                else
                {
                    pluginsDir.mkdir(pluginsPath);
                }
            }
        }
    }
}

void MainWindow::loadPlugin(QString pluginPath)
{
    BiliLiveDanmuToolPlugin *pluginClass;
    typedef BiliLiveDanmuToolPlugin *(*createPluginFun)();
    QString filePath = pluginPath;
    if(filePath.isEmpty()) return;
    if (QLibrary::isLibrary(filePath))
    {
        QLibrary danmukuPlugin(filePath);
        if (danmukuPlugin.load())
        {
            createPluginFun createPlugin = (createPluginFun)danmukuPlugin.resolve("createPlugin");
            if (createPlugin)
            {
                pluginClass = createPlugin();
                pluginsList.append(pluginClass);
            }
            else
            {
                qDebug() << danmukuPlugin.errorString();
            }
        }
    }
    else
    {
        qDebug() << "不是library";
    }
}

void MainWindow::startPlugin()
{
    if (selectedRowIndex > -1 && selectedRowIndex < pluginsList.size())
    {
        BiliLiveDanmuToolPlugin *pluginClass = pluginsList.at(selectedRowIndex);
        if (pluginClass->state() == BiliLiveDanmuToolPlugin::STOPPED)
        {
            connect(liveDanmuHandler,
                    SIGNAL(messageReceived(BiliLiveDanmuToolPlugin::ReceivedDanmukuProperties)),
                    pluginClass,
                    SLOT(ReceivedDanmuku(BiliLiveDanmuToolPlugin::ReceivedDanmukuProperties)));
            pluginClass->Start();
            tableModel.setItem(selectedRowIndex, 4, new QStandardItem("已启动"));
        }
    }
}

void MainWindow::stopPlugin()
{
    if (selectedRowIndex > -1 && selectedRowIndex < pluginsList.size())
    {
        BiliLiveDanmuToolPlugin *pluginClass = pluginsList.at(selectedRowIndex);
        if (pluginClass->state() == BiliLiveDanmuToolPlugin::RUNNING)
        {
            pluginClass->Stop();
            disconnect(liveDanmuHandler,
                    SIGNAL(messageReceived(BiliLiveDanmuToolPlugin::ReceivedDanmukuProperties)),
                    pluginClass,
                    SLOT(ReceivedDanmuku(BiliLiveDanmuToolPlugin::ReceivedDanmukuProperties)));
            tableModel.setItem(selectedRowIndex, 4, new QStandardItem("未启动"));
        }
    }
}

void MainWindow::managePlugin()
{
    if (selectedRowIndex > -1 && selectedRowIndex < pluginsList.size())
    {
        BiliLiveDanmuToolPlugin *pluginClass = pluginsList.at(selectedRowIndex);
        pluginClass->Setting();
    }
}

void MainWindow::showPluginMessage(QString title, QString content)
{
    appendLog(CUSTOM,
              "<span style=\"color:#f25d8e;\">" + title + "：</span>",
              "<span style=\"color:#f25d8e;\">" + content + "</span>");
}

void MainWindow::selectedRow(QModelIndex model)
{
    selectedRowIndex = model.row();
}

void MainWindow::notifyViewerChanged(int count)
{
    setWindowTitle("当前房间人数：" + QVariant(count).toString());
}

void MainWindow::handlerAction(Intent intent)
{
    switch (intent.getAction()) {
    case Intent::ACTION_ERROR_ALTER:
    {
        QMessageBox::critical(this, "错误", intent.getContent(), QMessageBox::Yes, QMessageBox::Yes);
        emit ui->pushButton_disconn->clicked();
        break;
    }
    case Intent::ACTION_RECONN:
    {
        if (ui->checkBox_autoReconn->isChecked())
        {
            appendLog(ERROR, "", "连接断开，3秒后自动重连...");
            reConnTimer->setSingleShot(true);
            reConnTimer->start(3000);
        }
        break;
    }
    case Intent::ACTION_UPDATE:
    {
        if (QMessageBox::information(this, "更新", "有新版本更新，需要打开浏览器下载吗？", QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes) == QMessageBox::Yes)
        {
            QDesktopServices::openUrl(QUrl(intent.getContent()));
        }
        break;
    }
    case Intent::ACTION_LOG_INFO:
    {
        appendLog(NORMAL, "", intent.getContent());
        break;
    }
    case Intent::ACTION_LOG_CUSTOM:
    {
        appendLog(CUSTOM, "", intent.getContent());
        break;
    }
    case Intent::ACTION_LOG_NOTICE:
     {
        appendLog(NOTICE, "", intent.getContent());
        break;
    }
    case Intent::ACTION_LOG_ERROR:
    {
        appendLog(ERROR, "", intent.getContent());
        emit ui->pushButton_disconn->clicked();
        break;
    }
    default:
        break;
    }
}

void MainWindow::sendBroadcast(BiliLiveDanmuToolPlugin::ReceivedDanmukuProperties danmuku)
{
    // log&notify
    switch (danmuku.danmukuType) {
    case BiliLiveDanmuToolPlugin::ReceivedDanmukuProperties::DANMUKU:
    {
        appendLog(DANMUKU, danmuku.userName, danmuku.danmukuText);
        popNotice(danmuku.userName, danmuku.danmukuText);
        danmukuCount++;
        break;
    }
    case BiliLiveDanmuToolPlugin::ReceivedDanmukuProperties::GIFT:
    {
        appendLog(GIFT, danmuku.userName, danmuku.giftName + "*" + QVariant(danmuku.giftNum).toString());
        popNotice(danmuku.userName, "赠送" + danmuku.giftName + "*" + QVariant(danmuku.giftNum).toString());
        giftCount += danmuku.giftNum;
        break;
    }
    case BiliLiveDanmuToolPlugin::ReceivedDanmukuProperties::WELCOME:
    {
        appendLog(WELCOME, "欢迎老爷", danmuku.userName);
        popNotice("欢迎老爷", danmuku.userName);
        break;
    }
    default:
        break;
    }
    ui->label_statistic->setText("收到弹幕" + QVariant(danmukuCount).toString() + "条，礼物" + QVariant(giftCount).toString() + "件");
}

void MainWindow::appendLog(int logType, QString title, QString content)
{
    QString time = "<span style=\"color:#808080;\">" + QDateTime::currentDateTime().toString("hh:mm:ss ") + "</span>";
    switch (logType) {
    case ERROR:
    {
        QString html = time + "<span style=\"color:#FF0000;\">" + title + " " + content + "</span>";
        ui->plainTextEdit_logs->appendHtml(html);
        break;
    }
    case NORMAL:
    {
        QString html = time + "<span style=\"color:#000000;\">" + title + " " + content + "</span>";
        ui->plainTextEdit_logs->appendHtml(html);
        break;
    }
    case CUSTOM:
    {
        QString html = time + title + content;
        ui->plainTextEdit_logs->appendHtml(html);
        html = "<div style=\"background-color: rgba(0,0,0,0.6)\">" + title + content + "</div>";
        emit messageGenerated(html);
        break;
    }
    case DANMUKU:
    {
        QString html = time
                + "<span style=\"color:#4fc1e9;\">" + title + "：</span>"
                + "<span style=\"color:#000000;\">" + content + "</span>";
        ui->plainTextEdit_logs->appendHtml(html);
        html = "<div style=\"background-color: rgba(0,0,0,0.6)\">"
               "<span style=\"color:" + scroller_unameColor + ";\">" + title + "：</span>"
                + "<span style=\"color:" + scroller_textColor + ";\">" + content + "</span></div>";
        emit messageGenerated(html);
        break;
    }
    case WELCOME:
    {
        QString html = time
                + "<span style=\"color:#ff8f34;\">" + title + "：</span>"
                + "<span style=\"color:#4fc1e9;\">" + content + "</span>"
                + "<span style=\"color:#000000;\"> 进入直播间</span>";
        ui->plainTextEdit_logs->appendHtml(html);
        html = "<div style=\"background-color: rgba(0,0,0,0.6)\">"
               "<span style=\"color:#ff8f34;\">" + title + "：</span>"
                + "<span style=\"color:" + scroller_unameColor + ";\">" + content + "</span>"
                + "<span style=\"color:" + scroller_textColor + ";\"> 进入直播间</span></div>";
        emit messageGenerated(html);
        break;
    }
    case GIFT:
    {
        QString html = time
                + "<span style=\"color:#ff8f34;\">礼物：</span>"
                + "<span style=\"color:#4fc1e9;\">" + title + "</span>"
                + "<span style=\"color:#000000;\"> 赠送的 " + content + "</span>";
        ui->plainTextEdit_logs->appendHtml(html);
        html = "<div style=\"background-color: rgba(0,0,0,0.6)\">"
               "<span style=\"color:#ff8f34;\">礼物：</span>"
               "<span style=\"color:" + scroller_unameColor + ";\">" + title + "</span>"
                + "<span style=\"color:" + scroller_textColor + ";\"> 赠送的 " + content + "</span></div>";
        emit messageGenerated(html);
        break;
    }
    case NOTICE:
    {
        QString html = time + "<span style=\"color:#f25d8e;\">" + title + " " + content + "</span>";
        ui->plainTextEdit_logs->appendHtml(html);
        break;
    }
    default:
        break;
    }
}

void MainWindow::popNotice(QString title, QString content)
{
    if (isNotify && isMinimized())
    {
        infoNotifier->show();
        infoNotifier->showMessage(title, content, QSystemTrayIcon::NoIcon);
    }
}

void MainWindow::autoScrolltoEnd()
{
    QTextCursor cursor = ui->plainTextEdit_logs->textCursor();
    cursor.movePosition(QTextCursor::End);
    ui->plainTextEdit_logs->setTextCursor(cursor);
}

void MainWindow::on_checkBox_keepTop_clicked(bool checked)
{
    if (checked)
        setWindowFlags(windowFlags() | Qt::WindowStaysOnTopHint);
    else
        setWindowFlags(windowFlags() &~ Qt::WindowStaysOnTopHint);
    show();
}

void MainWindow::on_checkBox_noticeOn_clicked(bool checked)
{
    isNotify = checked;
}

void MainWindow::on_pushButton_setFont_clicked()
{
    QFont fontUsing(scroller_fontFamily, scroller_fontSize);
    fontUsing.setStyleName(scroller_fontStyle);
    bool ok;
    QFont font = QFontDialog::getFont(
                    &ok, fontUsing, this);
    if (ok) {
        scroller_fontFamily = font.family();
        scroller_fontStyle = font.styleName();
        scroller_fontSize = font.pointSize();
        sideScroller->setScrollerFont(font);
    }
}

void MainWindow::on_pushButton_setColor_clicked()
{
    QColor unameColor;
    unameColor = QColorDialog::getColor(QColor(scroller_unameColor), this, "用户名称颜色", QColorDialog::DontUseNativeDialog);
    if (unameColor.isValid()) {
        scroller_unameColor = unameColor.name();
    }
    QColor textColor;
    textColor = QColorDialog::getColor(QColor(scroller_textColor), this, "弹幕文本颜色", QColorDialog::DontUseNativeDialog);
    if (textColor.isValid()) {
        scroller_textColor = textColor.name();
    }
}

void MainWindow::on_spinBox_stayTime_valueChanged(int time)
{
    sideScroller->setStayTimeLong(time*1000);
}

void MainWindow::on_checkBox_scollerOn_clicked(bool checked)
{
    if (checked)
        sideScroller->showView();
    else
        sideScroller->closeView();
}

void MainWindow::on_checkBox_scrollerResize_clicked(bool checked)
{
    if (checked)
        sideScroller->showWindowFrame(true);
    else
        sideScroller->showWindowFrame(false);
    if (ui->checkBox_scollerOn->isChecked())
        sideScroller->showView();
}
