/****************************************************************************
**
** Copyright (C) 2016 Kurt Pattyn <pattyn.kurt@gmail.com>.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtWebSockets module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** BSD License Usage
** Alternatively, you may use this file under the terms of the BSD license
** as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of The Qt Company Ltd nor the names of its
**     contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/
#include "ConnectServer.h"

ConnectServer::ConnectServer(quint16 port, QHostAddress address, QHostAddress sirenAddress, quint16 sirenPort, QObject *parent) :
    QObject(parent),
    m_pWebSocketServer(new QWebSocketServer(QStringLiteral("WebSocketfy"), QWebSocketServer::NonSecureMode, this))
{
    tcpSocket = new QTcpSocket(this);
    tcpSocket->connectToHost(sirenAddress, sirenPort);
    if (m_pWebSocketServer->listen(address, port) && tcpSocket->isOpen()) {
        std::cout << "WebSocketfy running on: " << address.toString().toStdString() << ", port: " << port << std::endl;
        std::cout << "Siren Server running on: " << sirenAddress.toString().toStdString() << ", port: " << sirenPort << std::endl;
        connect(m_pWebSocketServer, &QWebSocketServer::newConnection, this, &ConnectServer::onNewConnection);
        connect(m_pWebSocketServer, &QWebSocketServer::closed, this, &ConnectServer::closed);
        tcpSocket->close();
    } else {
        std::cout << "WebSocketfy lauching failed. Aborting..." << std::endl;
        return;
    }

    this->sirenAddress = sirenAddress;
    this->sirenPort = sirenPort;

    delete (tcpSocket);
    tcpSocket = nullptr;
}

ConnectServer::~ConnectServer(){

    m_pWebSocketServer->close();
    qDeleteAll(m_clients.begin(), m_clients.end());

    if (tcpSocket != nullptr){
        tcpSocket->close();
        delete (tcpSocket);
    }
}

void ConnectServer::onNewConnection(){

    QWebSocket *pSocket = m_pWebSocketServer->nextPendingConnection();

    connect(pSocket, &QWebSocket::binaryMessageReceived, this, &ConnectServer::processBinaryMessage);
    connect(pSocket, &QWebSocket::disconnected, this, &ConnectServer::socketDisconnected);
    m_clients << pSocket;
    std::cout << "New connection arrived! " << pSocket->peerAddress().toString().toStdString() << ":" << pSocket->peerPort() << std::endl;
}



void ConnectServer::processBinaryMessage(QByteArray message){

    QWebSocket *pClient = qobject_cast<QWebSocket *>(sender());
    QString input = message;
    QByteArray answer, buffer;
    QStringList tokens = input.split(" ");

    std::cout << "New request received: " << message.toStdString() << std::endl;

    //FTP-duty task
    if (tokens.first().toUpper() == "REQUEST"){

        if (tokens.size() > 1){
            QFile *file = new QFile("fs/" + tokens.at(1));
            std::cout << "Requested file: " + tokens.at(1).toStdString() << std::endl;
            if(file->open(QFile::ReadOnly)){
                answer = file->readAll();
                file->close();
            } else {
                answer = "File not found!";
            }
            delete file;
        } else {
            answer = "File name is missing!";
        }

    } else {
        //Close client connection by request
        if (tokens.first().toUpper() == "QUIT"){
            std::cout << "Client has disconnected. IP: " << pClient->peerAddress().toString().toStdString() << " PORT: " << pClient->peerPort() << std::endl;
            if (pClient) {
                m_clients.removeAll(pClient);
                pClient->deleteLater();
            }
        } else {
            //WebSocket <-> TCP Tunnel activation
            tcpSocket = new QTcpSocket(this);
            tcpSocket->connectToHost(sirenAddress, sirenPort);
            //Skip markers
            tcpSocket->waitForReadyRead(-1);
            tcpSocket->bytesAvailable();

            if (!tcpSocket->isOpen()){
                std::cout << "Fatal error. Siren Server not avaliable!" << std::endl;
                return;
            } else {
                //Send command (synchronous and blocking way)
                std::cout << "Sending TCP request. Please wait..." << std::endl;

                tcpSocket->write(input.toStdString().c_str());
                tcpSocket->waitForBytesWritten(-1);

                std::cout << "Waiting TCP reply. Please wait..." << std::endl;

                //Fetch answer (synchronous and blocking way)]
                tcpSocket->waitForReadyRead(-1);

                //Workaround loop for larger messages - Complete fix require implementing an application protocol
                //[#This issue related to the OS limits for the buffer size of tcp scokets - client side]
                //[#The server sends data with a single write, but multiple reads are required on the client side]
                while (tcpSocket->bytesAvailable() && (input.size() || !buffer.size())){
                    buffer = tcpSocket->readAll();
                    if (buffer.toStdString().empty()){
                        std::cout << "[WARNING]: Empty result set!" << std::endl;
                    }
                    input = buffer;

                    //Remove markers
                    input.replace("Siren::SQL> ", "");
                    if (input != "\n"){
                        answer = answer + input.toLocal8Bit();
                    } else {
                        input.clear();
                    }

                    tcpSocket->write("\n");
                    tcpSocket->waitForBytesWritten(-1);
                    tcpSocket->waitForReadyRead(-1);
                }

                tcpSocket->close();
                delete (tcpSocket);
                tcpSocket = nullptr;
            }
            //Tunnel ending
        }
    }
    std::cout << "WebSocketfy reply size: " << answer.size() << std::endl;
    if (pClient) {
        pClient->sendBinaryMessage(answer);
    }
}

void ConnectServer::socketDisconnected(){

    QWebSocket *pClient = qobject_cast<QWebSocket *>(sender());

    std::cout << "Client has disconnected. IP: " << pClient->peerAddress().toString().toStdString() << " PORT: " << pClient->peerPort() << std::endl;
    if (pClient) {
        m_clients.removeAll(pClient);
        pClient->deleteLater();
    }
}

