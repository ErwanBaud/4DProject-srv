#ifndef HEADER_SERVEUR
#define HEADER_SERVEUR

#include <QtWidgets>
#include <QtNetwork>
#include <set>
#include "ui_Serveur.h"
#include "Client.h"


class Serveur : public QWidget, private Ui::Serveur
{
    Q_OBJECT


    public:
        Serveur();
        QString hostPort; // Identité du serveur
        QList<Client *> clients; // Liste des processus clients

        Client *clientIsIn(Client *client, QList<Client *> &clients); // Vérifie la présence d'un client dans la liste
        //void envoyerATous(const QString &message);
        //void envoyerATous(const QString hostPort, const QTime time, const QString &message);
        //void envoyerAuxAutres(const QString &message);

    private slots:
        void clientAlive(); // Slot executé lors de la reception d'un iamAlive
        void whoIsAlive(); // Slot executé lors d'un clic sur le bouton whoIsAlive
        void deadCollector(); // Slot executé périodiquement pour modifier le statut des processus client n'emettant plus
        void startClient(); // Slot executé lors d'un clic sur le bouton lancer
        //void donneesRecues(); // Slot executé lors de la reception de données provenant d'un client
        void deconnexionClient(); // Slot executé lors de la deconnexion de la socket d'un client

    private:

        QHostAddress host; // Adresse IP du serveur
        quint16 port; // Port du serveur
        quint16 appPort; // Port de l'appli


        QTimer *tDeadCollector; // timer pour le slot deadCollector

        QUdpSocket *udpBroadSocket; // Socket d'ecoute des broadcasts iamAlive
        quint16 tailleMessage;

        void connectTo(Client *client);
};

#endif
