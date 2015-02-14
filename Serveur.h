#ifndef HEADER_SERVEUR
#define HEADER_SERVEUR

#include <QtWidgets>
#include <QtNetwork>
#include <QSignalMapper>
#include "ui_Serveur.h"
#include "Client.h"

class Serveur : public QWidget, private Ui::Serveur
{
    Q_OBJECT

    public:

        enum Ordre
        {
            START = 0,
            STOP,
            EXIT
        };

        Serveur();


    private:

        QHostAddress host; // Adresse IP du serveur
        quint16 port; // Port du serveur
        quint16 appPort; // Port de l'appli
        QString hostPort; // Identité du serveur

        QUdpSocket *udpBroadSocket; // Socket d'ecoute des broadcasts iamAlive
        QTimer *tDeadCollector; // timer pour le slot deadCollector


        QList<Client *> clients; // Liste des processus clients
        Client *clientIsIn(Client *client, QList<Client *> &clients); // Vérifie la présence d'un client dans la liste

        int checkClientsState(QList<Client *> listeClients); // Vérifie l'etat des clients connectés
        QList<Client *> clientsAlive();
        QList<Client *> clientsDead();

        void connectTo(Client *client);

        QSignalMapper *signalMapper; // SignalMapper pour les boutons Start / Stop
        quint16 tailleMessage;


    private slots:

        void receptionBroadcast(); // Slot executé lors de la reception d'un iamAlive

        void clientsState(); // Slot executé lors d'un clic sur le bouton State
        void deadCollector(); // Slot executé périodiquement pour modifier le statut des processus client n'emettant plus

        void startClient(); // Slot executé lors d'un clic sur le bouton lancer
        void deconnexionClient(); // Slot executé lors de la deconnexion de la socket d'un client

        void envoyerOrdre(int); // Envoi d'un ordre a un ou plusieurs client connecte
        void receptionClient(); // Slot executé lors de la reception de données provenant d'un client

        void writeLog(QString sender, QString log);
        void refreshButtons(int index);
};

#endif
