#include "Client.h"

Client::Client(){}

Client::Client(QString h, QString p)
{
    setHost(h);
    setPort(p);
    hostPort = host.toString() + ":" + QString::number(port);

    toClient = new QTcpSocket();
    status = 0;
}

QHostAddress Client::getHost()
{
    return host;
}

quint16 Client::getPort()
{
    return port;
}

void Client::setHost(QString h)
{
    if (!h.isEmpty()) host = QHostAddress(h);
}

void Client::setPort(QString p)
{
    if (!p.isEmpty()) port = p.toUShort();
}


bool operator==(const Client client1, const Client client2)
{
    bool r = false;
    if(client1.hostPort.compare(client2.hostPort) == 0) r = true;
    return r;
}


bool operator==(const Client client1, const Client *clientIterator)
{
    bool r = false;
    if(client1.hostPort.compare((*clientIterator).hostPort) == 0) r = true;
    return r;
}


uint qHash(const Client &c)
{
    return qHash(c.hostPort);
}
