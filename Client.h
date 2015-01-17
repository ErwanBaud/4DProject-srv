#ifndef CLIENT_H
#define CLIENT_H
#include <QtNetwork>

/* Client : Modelisation d'un processus client pour le serveur
 * */
class Client
{
    public:
        Client();
        Client(QString h, QString p);

        QString hostPort; // Identité du client
        quint16 status; // Statut :
                        // 0 -> dead
                        // 1 -> alive
                        // 2 -> connected


        QTime timeOut;  // timeout pour gèrer les deconnexions

        QHostAddress getHost();
        quint16 getPort();
        void setHost(QString h);
        void setPort(QString p);

        QTcpSocket *toClient; // Socket de communication vers le processus client

    private:
        QHostAddress host; // Adresse IP du client
        quint16 port; // Port du client
};

bool operator==(const Client client1, const  Client client2);
bool operator==(const Client client1, const  Client *clientIterator);
uint qHash(const Client &c);

#endif // Client_H
