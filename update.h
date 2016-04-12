#ifndef UPDATE_H
#define UPDATE_H

#include <QWidget>

#include <QNetworkAccessManager>
#include <QUrl>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QDir>

namespace Ui {
class Update;
}

class Update : public QWidget
{
    Q_OBJECT

public:
    explicit Update(QWidget *parent = 0);
    ~Update();

private:
    Ui::Update *ui;

    QNetworkAccessManager *netManager;

private slots:
    void replyFinished(QNetworkReply* reply);
    void on_pushButton_cancel_clicked();
};

#endif // UPDATE_H
