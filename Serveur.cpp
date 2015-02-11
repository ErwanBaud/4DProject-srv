#include "Serveur.h"
#include "ssh2_exec.h"

#define timeOut 3

/*****  Constructeur    ******/


Serveur::Serveur()
{
    setupUi(this);

    boutonStart->setEnabled(false);
    boutonStop->setEnabled(false);

    signalMapper = new QSignalMapper(this);

    connect(boutonQuitter, SIGNAL(clicked()), qApp, SLOT(quit()));
    connect(boutonWho, SIGNAL(clicked()), this, SLOT(clientsState()));
    connect(boutonLancer, SIGNAL(clicked()), this, SLOT(startClient()));

    connect(boutonStart, SIGNAL(clicked()), signalMapper, SLOT(map()));
    signalMapper->setMapping(boutonStart, START);
    connect(boutonStop, SIGNAL(clicked()), signalMapper, SLOT(map()));
    signalMapper->setMapping(boutonStop, STOP);

    connect(signalMapper, SIGNAL(mapped(int)), this, SLOT(envoyerOrdre(int)));

    //comboBoxSelection->InsertAlphabetically;
    comboBoxSelection->setMinimumWidth(110);
    comboBoxSelection->addItem("All");
    connect(comboBoxSelection , SIGNAL(currentIndexChanged(int)), this, SLOT(refreshButtons(int)));



    // Démarrage du serveur
    appPort = 50885;

    // Initialisation des composants pour la gestion des iamALive reçus
    udpBroadSocket = new QUdpSocket(this);
    if ( !(udpBroadSocket->bind(appPort, QUdpSocket::ShareAddress | QUdpSocket::ReuseAddressHint)) )
        listeMessages->append(tr("     Erreur socket UDP."));
    else
        //udpBroadSocket->bind(QHostAddress("127.0.0.1"), appPort);
        connect(udpBroadSocket, SIGNAL(readyRead()), this, SLOT(receptionBroadcast()));

        // Si le serveur a été démarré correctement
        host = QHostAddress("127.0.0.1");
        port = udpBroadSocket->localPort();
        hostPort = host.toString() + ":" + QString::number(port);

        // Mise à jour des labels de l'interface
        serveurIP->setText(host.toString());
        serveurPort->setText(QString::number(appPort));

        listeMessages->append(tr("Le serveur a ete demarre sur le port <strong>") + QString::number(port));

        // Initialisation des composants du deadCollector
        tDeadCollector = new QTimer();
        tDeadCollector->setInterval(3000);
        connect(tDeadCollector, SIGNAL(timeout()), this, SLOT(deadCollector()));
        tDeadCollector->start();

        tailleMessage = 0;
}




/*****  Broadcast   *****/


/* Executé a chaque reception d'un iamAlive
 * Ajout des nouveaux clients à la liste
 * Rafraichissement des timeout
 * */
void Serveur::receptionBroadcast()
{
    // Tant qu'il y a des paquets reçus
     while (udpBroadSocket->hasPendingDatagrams())
     {
        //listeMessages->append("Client alive.");

        QByteArray datagram;
        QList<QByteArray> mess;
        datagram.resize(udpBroadSocket->pendingDatagramSize());
        udpBroadSocket->readDatagram(datagram.data(), datagram.size());
        mess = datagram.split('#');
        Client *client = new Client(mess[0], mess[1]); // Création d'un objet client
        Client *pClient = clientIsIn(client, clients); // Vérification de la présence dans la liste

        bool b = (mess[3] == "1" ? true : false);

        // S'il n'y est pas
        if ( pClient == NULL )
        {
             //listeMessages->append("Absent.");
             client->alive = 1; // Statut = alive
             client->state = b;
             client->timeStamp = QTime::currentTime();
             connectTo(client);
             clients.append(client);

             // Ajout dans la combobox
             comboBoxSelection->addItem(client->hostPort);
        }
        // S'il y est
        else
        {
            //listeMessages->append("Present.");
            // Refresh timeout du client trouvé
            (*pClient).timeStamp = QTime::currentTime();
            (*pClient).state = b;

            if((*pClient).alive == 0)
            {
                (*pClient).alive = 1; // Statut = alive

                // Ajout dans la combobox
                comboBoxSelection->addItem(client->hostPort);

                listeMessages->append((*pClient).hostPort + " retrouve.");
            }
        }
    }
    refreshButtons(comboBoxSelection->currentIndex());
}




/***** Client  *****/


/* Retourne un pointeur vers le client s'il est dans la liste
 * Retourne un pointeur NULL sinon
 * */
Client * Serveur::clientIsIn(Client *client, QList<Client *> &clients)
{
    QList<Client *>::Iterator clientIterator = clients.end();
    for( clientIterator = clients.begin(); clientIterator != clients.end(); ++clientIterator )
        if( *client == *clientIterator )
            return *clientIterator;

    return NULL;
}


/* Affiche les clients contenu dans la liste du serveur
 *  */
void Serveur::clientsState()
{
    QList<Client *> listAlive = clientsAlive();
    QList<Client *> listDead = clientsDead();

    listeMessages->append("");

    listeMessages->append("Are alive on this server :");
    listeMessages->append("   Etat collectif des alive: " + QString::number(checkClientsState(clients)));
    for(QList<Client *>::Iterator clientIterator = listAlive.begin(); clientIterator != listAlive.end(); ++clientIterator )
    {
            listeMessages->append("\t" + (*clientIterator)->hostPort
                                  + "  alive: " + QString::number((*clientIterator)->alive)
                                  + "  state: " + QString::number((*clientIterator)->state)
                                  + "  timeOut: " + (*clientIterator)->timeStamp.toString() + "s.");
    }
    listeMessages->append("");

    listeMessages->append("Are dead on this server :");
    for(QList<Client *>::Iterator clientIterator = listDead.begin(); clientIterator != listDead.end(); ++clientIterator )
    {
            listeMessages->append("\t" + (*clientIterator)->hostPort
                                  + "  alive: " + QString::number((*clientIterator)->alive)
                                  + "  state: " + QString::number((*clientIterator)->state)
                              + "  timeOut: " + (*clientIterator)->timeStamp.toString() + "s.");
    }
    listeMessages->append("");
}


/* Vérifie l'etat des clients alive
 * Renvoie 0 s'ils sont tous STOP
 *         1 s'ils sont tous START
 *         -1 si pas dans le meme etat
 * */
int Serveur::checkClientsState(QList<Client *> listeClients)
{
    int res = -1;

    if( listeClients.size() > 0 )
    {
        int b = 0;

        for (int i = 0; i < listeClients.size(); i++)
        {
            b += listeClients[i]->state;
        }

        if(b == 0) res = 0;
        if(b == listeClients.size()) res = 1;
    }

    return res;
}


/* Renvoie la liste des clients "Alive"
 * */
QList<Client *> Serveur::clientsAlive()
{
    QList<Client *> listAlive;

    for(QList<Client *>::Iterator clientIterator = clients.begin(); clientIterator != clients.end(); ++clientIterator )
        if( (*clientIterator)->alive ) listAlive << *clientIterator;

    return listAlive;
}


/* Renvoie la liste des clients "Dead"
 * */
QList<Client *> Serveur::clientsDead()
{
    QList<Client *> listDead;

    for(QList<Client *>::Iterator clientIterator = clients.begin(); clientIterator != clients.end(); ++clientIterator )
        if( !(*clientIterator)->alive ) listDead << *clientIterator;

    return listDead;
}


/* Actualisation des statuts des clients
 * Actualise la combobox
 * */
void Serveur::deadCollector()
{
    for(QList<Client *>::Iterator clientIterator = clients.begin(); clientIterator != clients.end(); ++clientIterator )
        if( (*clientIterator)->alive != 0 )
            if( (*clientIterator)->timeStamp < QTime::currentTime().addSecs(-timeOut))
            {
                (*clientIterator)->alive = 0;
                // Suppresion de l'entrée dans la combobox
                comboBoxSelection->removeItem(comboBoxSelection->findText((*clientIterator)->hostPort));
                listeMessages->append("\t" + (*clientIterator)->hostPort + " is dead !");
            }
}




/* Lance un client sur 127.0.0.1 et un port aléatoire
 *  */
void Serveur::startClient()
{    
    //QString program = QDir::home().filePath("4DProject/deploy/cltCore.exe");
    /*QProcess *myProcess = new QProcess(this);
    if(myProcess->startDetached(program))
    {
        listeMessages->append("");
        listeMessages->append("Client démarré !");
        listeMessages->append("");
    }
    else
    {
        listeMessages->append("");
        listeMessages->append("Client non démarré !");
        listeMessages->append("");
    }*/
    if(!ssh_exec_command("127.0.0.1","projet4d","nohup 4DProject/deploy/cltCore.exe  > /dev/null 2> /dev/null </dev/null &"))
    {
        listeMessages->append("");
        listeMessages->append("Client démarré !");
        listeMessages->append("");
    }
    else
    {
        listeMessages->append("");
        listeMessages->append("Client non démarré !");
        listeMessages->append("");
    }


}


/* Ouverture d'une socket hyperviseur-client
 * */
void Serveur::connectTo(Client *client)
{
    listeMessages->append("    Tentative de connexion a " + client->hostPort);

    connect(client->toClient, SIGNAL(readyRead()), this, SLOT(receptionClient()));
    connect(client->toClient, SIGNAL(disconnected()), this, SLOT(deconnexionClient()));


    QString clientKey = QDir::home().filePath("4DProject/4DProject-cert/client/client-key.pem");
    QString clientCrt = QDir::home().filePath("4DProject/4DProject-cert/client/client-crt.pem");
    QString ca = QDir::home().filePath("4DProject/4DProject-cert/ca/ca.pem");

    //on charge la clé privé du client
    QFile file(clientKey);

    if( ! file.open( QIODevice::ReadOnly ) )
    {
        qDebug() << "Erreur: Ouverture de la cle impossible (" << file.fileName() << ") : " << file.errorString();
        return;
    }
    QSslKey key(&file, QSsl::Rsa, QSsl::Pem, QSsl::PrivateKey, "4DProject");
    file.close();
    client->toClient->setPrivateKey( key );

    //on charge le certificat du client
    client->toClient->setLocalCertificate(clientCrt);

    //on charge le certificat de notre ca
    if( !client->toClient->addCaCertificates(ca))
    {
        qDebug() << "Erreur: Ouverture du certificat de CA impossible (" << ca << ")";
        return;
    }
    //on supprime la vérification du serveur
    client->toClient->setPeerVerifyMode(QSslSocket::VerifyNone);

    //on ignore les erreurs car on a un certificat auto signé
    client->toClient->ignoreSslErrors();

    // On désactive les connexions précédentes s'il y en a
    client->toClient->abort();

    //on se connecte au serveur
    client->toClient->connectToHostEncrypted(client->getHost().toString(), client->getPort());
    client->toClient->waitForEncrypted();

    if( client->toClient->isOpen() && client->toClient->isEncrypted())
        listeMessages->append("    Connecte a " + client->hostPort);
}


/* Deconnexion de la socket d'un client
 * */
void Serveur::deconnexionClient()
{
    // On détermine quel client se déconnecte
    QTcpSocket *socket = qobject_cast<QTcpSocket *>(sender());
    if (socket == 0) // Si par hasard on n'a pas trouvé le client à l'origine du signal, on arrête la méthode
        return;


    foreach(Client *c, clients)
        if( c->toClient == socket )
        {
            c->toClient->close();
            c->toClient->deleteLater();
            listeMessages->append("    /!\\" + c->hostPort + " vient de se deconnecter.");
            break;
        }
}




/* Envoi d'un ordre a tous les clients
 * */
void Serveur::envoyerOrdre(int pOrdre)
{
    Ordre ordre = (Ordre)pOrdre;
    QList<Client *> listAlive;
    int ccs = -1, index = comboBoxSelection->currentIndex();

    if( index == 0 )
    {
        listAlive = clientsAlive();
        ccs = checkClientsState(listAlive);
        listeMessages->append("Envoye ordre " + QString::number(pOrdre) + " a tous les clients ALIVE.");
    }
    else
    {
        QList<Client *>::Iterator clientIterator = clients.begin();
        while( (*clientIterator)->hostPort != comboBoxSelection->itemText(index))
            ++clientIterator;

        listAlive.append(*clientIterator);
        ccs = (*clientIterator)->state;
        listeMessages->append("Envoye ordre " + QString::number(pOrdre) + " a " + (*clientIterator)->hostPort);
    }

    // Si les clients sont dans un etat incoherent
    if( pOrdre != ccs )
    {
        listeMessages->append("L'etat des clients alive est incoherent, tous vont etre STOP.");
        ordre = STOP;
    }


    // Préparation du paquet
    QByteArray paquet;
    QDataStream out(&paquet, QIODevice::WriteOnly);

    out << (quint16) 0; // On écrit 0 au début du paquet pour réserver la place pour écrire la taille
    out << hostPort << QTime::currentTime() << ordre;
    out.device()->seek(0); // On se replace au début du paquet
    out << (quint16) (paquet.size() - sizeof(quint16)); // On écrase le 0 qu'on avait réservé par la longueur du message


    // Envoi du paquet préparé à tous les clients connectés au serveur
    for(QList<Client *>::Iterator clientIterator = listAlive.begin(); clientIterator != listAlive.end(); ++clientIterator )
        (*clientIterator)->toClient->write(paquet);
}


/* Reception de données provenant d'un client
 * */
void Serveur::receptionClient()
{
    QTcpSocket *socket = qobject_cast<QTcpSocket *>(sender());
    if (socket == 0) // Si par hasard on n'a pas trouvé le client à l'origine du signal, on arrête la méthode
        return;

    // Si tout va bien, on continue : on récupère le message
    QDataStream in(socket);

    if (tailleMessage == 0) // Si on ne connaît pas encore la taille du message, on essaie de la récupérer
    {
        if (socket->bytesAvailable() < (int)sizeof(quint16)) // On n'a pas reçu la taille du message en entier
             return;

        in >> tailleMessage; // Si on a reçu la taille du message en entier, on la récupère
    }

    // Si on connaît la taille du message, on vérifie si on a reçu le message en entier
    if (socket->bytesAvailable() < tailleMessage) // Si on n'a pas encore tout reçu, on arrête la méthode
        return;


    // Si on arrive jusqu'à cette ligne, on peut récupérer le message entier
    QString sender, messageRecu;
    QTime time;
    in >> sender;
    in >> time;
    in >> messageRecu;

    listeMessages->append(time.toString() + " - Recu de " + sender + " : " + messageRecu);

    // 3 : remise de la taille du message à 0 pour permettre la réception des futurs messages
    tailleMessage = 0;
}



/*****  Interface   *****/


/* Rafraichit l'affichage des boutons START / STOP
 * */
void Serveur::refreshButtons(int index)
{
    int ccs = -1;

    if( index == 0 )
        ccs = checkClientsState(clientsAlive());
    else
    {
        QList<Client *>::Iterator clientIterator = clients.begin();
        while( (*clientIterator)->hostPort != comboBoxSelection->itemText(index))
            ++clientIterator;
        ccs = (*clientIterator)->state;
    }

    // Tous STOP
    if( ccs == 0 )
    {
            boutonStart->setEnabled(true);
            boutonStop->setEnabled(false);
    }
    // Tous START
    else if ( ccs == 1 )
    {
            boutonStart->setEnabled(false);
            boutonStop->setEnabled(true);
    }
    //Etat incoherent
    else
    {
            boutonStart->setEnabled(true);
            boutonStop->setEnabled(true);
    }
}
