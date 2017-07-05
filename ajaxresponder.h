#ifndef AJAXRESPONDER_H
#define AJAXRESPONDER_H

#include <QObject>
#include <QThread>

#include <QTcpSocket>
#include <QTcpServer>

class AjaxResponderThread : public QThread
{
    Q_OBJECT

public:
    AjaxResponderThread(int socketDescriptor, const QString &fortune, QObject *parent);

    void run() override;

signals:
    void error(QTcpSocket::SocketError socketError);

private:
    int socketDescriptor;
    QString text;
};




class AjaxResponder : public QTcpServer
{
    Q_OBJECT


public:
    AjaxResponder(QObject *parent = 0);

protected:
    void incomingConnection(qintptr socketDescriptor) override;

private:
    QStringList fortunes;
};

#endif // AJAXRESPONDER_H
