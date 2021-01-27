#include <QMessageBox>
#include "logindialog.h"
#include "ui_logindialog.h"

LoginDialog::LoginDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::LoginDialog)
{
    ui->setupUi(this);
    setAttribute(Qt::WA_DeleteOnClose, true);
    setWindowFlag(Qt::WindowContextHelpButtonHint, false);

    ui->stackedWidget->setCurrentIndex(0);
}

LoginDialog::~LoginDialog()
{
    delete ui;
}

void LoginDialog::on_loginButton_clicked()
{
    if (ui->neteaseRadio->isChecked())
    {
        QString username = ui->usernameEdit->text().trimmed();
        QString password = ui->passwordEdit->text().trimmed();
        if (username.isEmpty() || password.isEmpty())
            return ;
        loginNetease(username, password);
    }
    else if (ui->neteaseCookieRadio->isChecked())
    {
        QString cookies = ui->cookieEdit->toPlainText();
        if (cookies.isEmpty())
            return ;
        cookieNetease(cookies);
    }
    else if (ui->qqmusicCookieRadio->isChecked())
    {
        QString cookies = ui->cookieEdit->toPlainText();
        if (cookies.isEmpty())
            return ;
        cookieQQMusic(cookies);
    }
}

void LoginDialog::loginNetease(QString username, QString password)
{
    password = password.toLocal8Bit().toPercentEncoding();
    bool usePhone = (username.indexOf("@") == -1);
    QString url = NETEASE_SERVER + (usePhone
            ? "/login/cellphone?phone=" + username +"&password=" + password
            : "/login?email=" + username + "&password=" + password);

    QNetworkAccessManager* manager = new QNetworkAccessManager;
    QNetworkRequest* request = new QNetworkRequest(url);
    request->setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded; charset=UTF-8");
    request->setHeader(QNetworkRequest::UserAgentHeader, "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/86.0.4240.111 Safari/537.36");
    connect(manager, &QNetworkAccessManager::finished, this, [=](QNetworkReply* reply){
        QJsonParseError error;
        QByteArray ba = reply->readAll();
        QJsonDocument document = QJsonDocument::fromJson(ba, &error);
        if (error.error != QJsonParseError::NoError)
        {
            qDebug() << error.errorString();
            return ;
        }
        QJsonObject json = document.object();
        int code = json.value("code").toInt();
        if (code != 200)
        {
            QMessageBox::warning(this, "登录错误", "账号或密码错误，请重试\n错误码：" + QString::number(code));
            return ;
        }

        if(reply->hasRawHeader("Set-Cookie"))
        {
            QByteArray cookie = reply->rawHeader("Set-Cookie");
            qDebug() << "cookie:" << cookie;
            qDebug() << "data:" << ba;
            emit signalLogined(NeteaseCloudMusic, cookie);
        }

        manager->deleteLater();
        delete request;
        reply->deleteLater();
        this->close();
    });
    manager->get(*request);
}

void LoginDialog::cookieNetease(QString cookies)
{
    emit signalLogined(NeteaseCloudMusic, cookies);
    this->close();
}

void LoginDialog::cookieQQMusic(QString cookies)
{
    emit signalLogined(QQMusic, cookies);
    this->close();
}

void LoginDialog::on_neteaseRadio_clicked()
{
    ui->stackedWidget->setCurrentIndex(0);
}

void LoginDialog::on_cookieHelpButton_clicked()
{
    QString text = "目前只支持用户自己从 网易云音乐/QQ音乐 的网页版获取获取 Cookie。\n\n步骤：\n浏览器登录按 F12 打开开发者工具, 找到 Network\n随便进入网易云音乐/QQ音乐的网页, 复制右边 request headers 中的 Cookie";
    QMessageBox::information(this, "获取Cookies", text);
}

void LoginDialog::on_neteaseCookieRadio_clicked()
{
    ui->stackedWidget->setCurrentIndex(1);
}

void LoginDialog::on_qqmusicCookieRadio_clicked()
{
    ui->stackedWidget->setCurrentIndex(1);
}
