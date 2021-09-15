//Qt includes
#include <QCoreApplication>

//Local includes
#include <ConnectServer.h>

//Std includes
#include <iostream>


int main(int argc, char *argv[]){

    QCoreApplication a(argc, argv);

    QHostAddress addressSiren, address;
    QString hostnameSiren, hostname;

    int port, sirenPort;
    port = sirenPort = -1;

    for (int x = 1; x < argc-1; x++){
        QString aux = argv[x];
        if (aux.toUpper() == "-HI"){
            hostname = argv[x+1];
            x++;
        } else {
            if (aux.toUpper() == "-PI"){
                port = QString(argv[x+1]).toInt();
                x++;
            } else {
                if (aux.toUpper() == "-PS"){
                    sirenPort = QString(argv[x+1]).toInt();
                    x++;
                } else {
                    if (aux.toUpper() == "-HS"){
                        hostnameSiren = argv[x+1];
                        x++;
                    } else {
                        std::cout << "Invalid server setup. Exiting... \n" << std::endl;
                        return 0;
                    }
                }
            }
        }
    }


    if (hostname.isEmpty()){
        std::string aux;
        std::cout << "Hostname/IP: ";
        std::cin >> aux;
        hostname.fromStdString(aux);
    }

    if (hostnameSiren.isEmpty()){
        std::string aux;
        std::cout << "Siren Server Name/ip: ";
        std::cin >> aux;
        hostnameSiren.fromStdString(aux);
    }

    if (port == -1){
        std::string aux;
        std::cout << "Port: ";
        std::cin >> aux;
        port = QString::fromStdString(aux).toInt();
    }

    if (sirenPort == -1){
        std::string aux;
        std::cout << "Siren port: ";
        std::cin >> aux;
        sirenPort = QString::fromStdString(aux).toInt();
    }


    address.setAddress(hostname);
    addressSiren.setAddress(hostnameSiren);
    ConnectServer server(port, address, addressSiren, sirenPort);
    QObject::connect(&server, &ConnectServer::closed, &a, &QCoreApplication::quit);

    return a.exec();
}
