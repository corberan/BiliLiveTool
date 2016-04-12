#include "update.h"
#include "ui_update.h"

Update::Update(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Update)
{
    ui->setupUi(this);

    netManager = new QNetworkAccessManager(this);
    connect(netManager, SIGNAL(finished(QNetworkReply*)), this, SLOT(replyFinished(QNetworkReply*)));

//    QString url = "http://live.bilibili.com/" + QVariant(this->roomID).toString();
//    manager->get(QNetworkRequest(QUrl(url)));
    ui->plainTextEdit_info->appendPlainText(QDir::currentPath());
}

Update::~Update()
{
    delete ui;
}

void Update::replyFinished(QNetworkReply *reply)
{

}

void Update::on_pushButton_cancel_clicked()
{
    close();
}
