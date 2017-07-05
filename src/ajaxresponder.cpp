#include "ajaxresponder.h"

#include <QDataStream>

AjaxResponder::AjaxResponder(QObject *parent)
{

    fortunes << tr("You've been leading a dog's life. Stay off the furniture.")
             << tr("You've got to think about tomorrow.")
             << tr("You will be surprised by a loud noise.")
             << tr("You will feel hungry again in another hour.")
             << tr("You might have mail.")
             << tr("You cannot kill time without injuring eternity.")
             << tr("Computers are not intelligent. They only think they are.");
}


void AjaxResponder::incomingConnection(qintptr socketDescriptor)
{
    QString fortune = fortunes.at(qrand() % fortunes.size());

    AjaxResponderThread *thread = new AjaxResponderThread(socketDescriptor, fortune, this);

    connect(thread, SIGNAL(finished()), thread, SLOT(deleteLater()));

    thread->start();

}

AjaxResponderThread::AjaxResponderThread(int socketDescriptor, const QString &fortune, QObject *parent)
    : QThread(parent), socketDescriptor(socketDescriptor), text(fortune)
{
}

void AjaxResponderThread::run()
{
    QTcpSocket tcpSocket;

    if (!tcpSocket.setSocketDescriptor(socketDescriptor)) {
        emit error(tcpSocket.error());
        return;
    }

    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_4_0);
    out << text;

    tcpSocket.write(block);
    tcpSocket.disconnectFromHost();
    tcpSocket.waitForDisconnected();

}
