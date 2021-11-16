#include "SirenTransferProtocol.h"

SirenTransferProtocol::SirenTransferProtocol(){
}

void SirenTransferProtocol::send(QTcpSocket* destiny, QString message){

    //Sync sending...
    //First, send message size ...
    destiny->write(QByteArray(QString::number(message.toUtf8().size()).toUtf8()));
    destiny->waitForBytesWritten();
    //Then, send data message ...
    destiny->write(message.toUtf8());
    destiny->waitForBytesWritten();
}

QString SirenTransferProtocol::receive(QTcpSocket* origin){

    QString answer;
    QByteArray df = origin->readAll();
    long int data = QString(df).toLong();

    while (data > 0){
        origin->waitForReadyRead();
        df = origin->readAll();
        answer += QString::fromUtf8(df);
        data -= df.size();
    }

    return answer;
}
