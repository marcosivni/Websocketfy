#ifndef SIRENTRANSFERPROTOCOL_H
#define SIRENTRANSFERPROTOCOL_H

#include<QTcpSocket>

class SirenTransferProtocol{

    public:
        SirenTransferProtocol();

        void send(QTcpSocket* destiny, QString message);
        QString receive(QTcpSocket* origin);
};

#endif // SIRENTRANSFERPROTOCOL_H
