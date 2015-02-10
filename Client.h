#ifndef CLIENT_H
#define CLIENT_H
#include <QtNetwork>



/* Client : Modelisation d'un processus client pour le serveur
 * */
class Client
{
    public:

        enum Ordre
        {
            START = 0,
            STOP
        };

        Client();
        Client(QString h, QString p);

        QHostAddress getHost();
        quint16 getPort();
        void setHost(QString h);
        void setPort(QString p);

        QString hostPort; // Identité du client

        bool alive; // Broadcast reçus (true) ou non (false)
        bool state; // Simu lancée (true) ou non (false)

        QSslSocket *toClient; // Socket de communication vers le processus client

        QTime timeStamp;  // timeout pour gèrer les deconnexions

    private:
        QHostAddress host; // Adresse IP du client
        quint16 port; // Port du client
};

bool operator==(const Client client1, const  Client client2);
bool operator==(const Client client1, const  Client *clientIterator);
uint qHash(const Client &c);

#endif // Client_H
