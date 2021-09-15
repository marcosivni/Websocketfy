# Websocketfy Project for Higiia

## Tunneling for extended SQL and filesystem access

The Websocketfy Server in this repository was designed to:

1. Tunnel extended SQL commands to a [SiREN Server](https://github.com/marcosivni/siren),
2. Accessing and transfering medical image files stored into the (Websocketfy server side) filesystem.

One may argue [SiREN](https://github.com/marcosivni/siren) and an [FTP](https://security.appspot.com/vsftpd.html) servers are the most suitable and direct service-oriented solutions for performing the tasks above, so *why the need for a Websocketfy Server*?

Notice they both execute application protocols based on reliable TCP connections in the transport layer of the TCP/IP network model. Accordingly, the client side of those servers must support TCP sockets for opening and maintaing connections.
This is no problem for the major part of clients that rely on the operating system API for stablishing TCP connections, such as Phyton scripts, Java-based/php websites, or C++ desktop applications (running [QTCPSocket](https://doc.qt.io/qt-5/qtcpsocket.html), for instance). 
However, WebAssembly *does not* offer direct support for TCP connections due to ~~some [oddly reasons](https://github.com/bytecodealliance/wasmtime/issues/70)~~ its sandbox design. In particular, no standard support for that issue is provided by the QT SDK (See [here](https://bugreports.qt.io/browse/QTBUG-63920)), which is the platform Higiia is build on.

Accordingly, the ~~workaround~~ solution we can use is "websocketfy" the communication between the cliente and those servers, which means using an *application-layer* protocol named [WebSocket](https://datatracker.ietf.org/doc/html/rfc6455) to stablish a connection exchange control messages with *another* application-layer protocol. 

QT SDK with Emscripten offers support for WebSocket through [QWebSocket](https://doc.qt.io/qt-5/qwebsocket.html), which we used in server implementation in this repository (Qt SDK 5.15.1 - LGPL-license, Emscripten with em++ compiler v2.0.22). Besides the tunneling of extended SQL commands to SIREN, we implemented a basic (download-only) transfering file functionality rather than a second tunnel to an FTP Server within our Websocketfy-Server. Such functionality is available by using reserved word `REQUEST` within an aplication message (See [here](www) for an example).

> **NOTE:** Security-driven protocols (*e.g.,* SSL, TLS, etc.) are currently [not fully supported](https://webassembly.org/docs/security/) by WebAssembly. They are expected to be implemented in the near-future.

## Client-Server architecture with the Websocketfy Server

The Websocketfy Server is a middle piece between the Higiia cliente and the extended SQL server. Since the client typical request (very) large image files, we opt to keep those files organized in an external filesystem (that can be shared with PACS environments) accessible to the Websocketfy Server to shorten trasnfering distances and reduce overhead in this client-server architecture. Accordingly, the Websocketfy Server is also responsible for transfering the files to the client, urburdening the SIREN server. Figure 1 summarizes the relationships between the Higiia client and the data servers.

![Higiia relationships](https://github.com/marcosivni/higiia/blob/main/model/example/imgs/architecture.png)
Figure 1. Relationships between the Higiia client and data servers.

#Installation 


