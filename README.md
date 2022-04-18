# Websocketfy Project for WebHigiia

## Tunneling for extended SQL and filesystem access

The Websocketfy-Server in this repository was designed to:

1. Tunnel extended SQL commands to a [SiREN Server](https://github.com/marcosivni/siren),
2. Accessing and transfering medical image files stored into the (Websocketfy server side) filesystem.

One may argue [SiREN](https://github.com/marcosivni/siren) and [FTP](https://security.appspot.com/vsftpd.html) servers are the most suitable and direct service-oriented solutions for performing the tasks above, so *why the need for a Websocketfy-Server*?

Notice they both execute application protocols based on reliable TCP connections in the transport layer of the TCP/IP network model, so the client-side of those servers must support TCP sockets for opening and maintaining network connections.
This is no problem for clients that rely on the operating system API for establishing TCP connections, such as Phyton scripts, Java-based/PHP web servers, or C++ desktop applications (running [QTCPSocket](https://doc.qt.io/qt-5/qtcpsocket.html), for instance). 
However, WebAssembly *does not* offer direct support for TCP connections due to ~~some [odd reasons](https://github.com/bytecodealliance/wasmtime/issues/70)~~ its sandbox design. In particular, no standard support for that issue is provided by the QT SDK (See [here](https://bugreports.qt.io/browse/QTBUG-63920)), which is the platform where WebHigiia is coded.

Accordingly, the ~~workaround~~ solution we can use is "websocketfy" the communication between the client and those servers, which means using an *application-layer* protocol named [WebSocket](https://datatracker.ietf.org/doc/html/rfc6455) to establish a connection and exchange control messages with *another* application-layer protocol. 

Qt SDK with Emscripten offers support for WebSocket through [QWebSocket](https://doc.qt.io/qt-5/qwebsocket.html), which we used in in this repository (Qt SDK 5.15.1 - LGPL-license, Emscripten with em++ compiler v2.0.22). In addition to the tunneling of extended SQL commands to SIREN, we implemented a basic (download-only) transferring file functionality rather than a second tunnel to an FTP Server within our Websocketfy-Server. Such functionality is available by using the reserved word `REQUEST` within an application message.

> **NOTE:** Security-driven protocols (*e.g.,* SSL, TLS, etc.) are currently [not fully supported](https://webassembly.org/docs/security/) by WebAssembly. They are expected to be implemented in the near future.

## Client-Server architecture with the Websocketfy-Server

The Websocketfy-Server is a middle piece between the WebHigiia client and the extended SQL server. Since the client typically requests large image files, we opt to keep those files organized in an external filesystem (shared with PACS environments) accessible to the Websocketfy-Server to shorten transferring distances and reduce overhead in this client-server architecture. Accordingly, the Websocketfy-Server is also responsible for conveying the files to the client, unburdening the SIREN server. Figure 1 summarizes the relationships between the Higiia client and the data servers.

![Higiia relationships](https://github.com/marcosivni/higiia/blob/main/model/example/imgs/architecture.png)
Figure 1. Relationships between the Higiia client and data servers.

## Installation 

Websocketfy-Server depends on QT SDK and requires both QT `network` and `websockets` modules to be installed, see the guide below.

### Downloading background technologies

- **QT SDK** - Available [here](https://www.qt.io/download) under LGPL license.

> When compiling SIREN over a Debian-based Linux distro, *e.g.*, [Ubuntu](https://ubuntu.com/) or [QLustar](https://qlustar.com/), you can use the package manager for downloading the latest QT SDK. Historically, the package name follows the structure `qtX-default`, where `X` stands for the QT version. The following command enables the installation of the QT 5 package (see Figure 1). Please, remember to check for deprecated as you read this guide.

```console
# apt-get update && apt-get dist-upgrade
# apt-get -f install qt5-default
```

![Figure 1. Getting QT SDK 5 on Debian-based systems.](https://github.com/marcosivni/siren/blob/main/example/InstallQt-P1.png)
Figure 1. Getting QT SDK 5 on Debian-based systems.

> The `network` module is a default package downloaded with qt5-default.

- **Qt Web Sockets module** - Avaliable [here](https://www.qt.io/).  

> Alternatively, you can install Qt Web Sockets module by using the package manager of your Linux operating system, as in the Debian-based command below. Please, be aware of checking for deprecated packages.

```console
# apt-get -f install libqt5websockets5-dev
```

### Bringing the pieces together

Once we installed the background technologies, Websocketfy-Server is ready to be compiled. We just need to indicate the QT modules in the "soon-to-be Makefile" `.pro` file. The `WS-Server.pro.example` file provides the basis for those indications.

1. Create a `WS-Server.pro` entry based on the `WS-Server.pro.example` file.

```console
$ cp WS-Server.pro.example WS-Server.pro
``` 

2. Locate the QT directives and modify them (if necessary).

```vim
QT += network
QT += websockets
```  

3. Run **qmake**. Run the binary qmake (produced after the QT installation) to create a `Makefile` from the `WS-Server.pro` file.

```console
$ cd Websocket-Server-path/websocketfy/
$ qmake
```  

### Generating the binary

We can compile Websocketfy-Server after the `Makefile` generation, which will produce the executable file `WS-Server`.

1. Run `make` 

```console
$ cd Websocket-Server-path/websocketfy/
$ make && make clean
```

2. Move the binary to a proper location. We suggest creating/using a separated directory `\bin`.

```console
$ cd Websocket-Server-path/
$ mkdir bin
$ mv websocketfy/WS-Server bin/WS-Server
```

3. Create a directory named `fs` to serve as the entry point to the Websocketfy-Server file path. Image files must be saved in (any level below) the `fs` directory.

```console
$ cd Websocket-Server-path/bin
$ mkdir fs
```


### Running the Websocketfy-Server

The Websocketfy-Server binary requires a set of arguments, namely:

- `-hi` - The host address (IP) for the Websocketfy-Server (mandatory field).
- `-pi` - The port (socket) for the Websocketfy-Server process (mandatory field).
- `-hs` - The host address (IP) of the SIREN Server (mandatory field).
- `-ps` - The port (socket) of the SIREN Server process (mandatory field).


A call for the Websocketfy-Server (binary `WS-Binary`) running in localhost on socket `3333` is as follows.

```/bin/bash
./WS-Server -hi 127.0.0.1 -pi 3333 -hs 127.0.0.1 -ps 3434
```

> **NOTE:** We assume SIREN Server is also running on localhost port 3434 - [Example](https://github.com/marcosivni/siren/wiki/Server-Setup).

Figure 2 presents the expected output for a valid  Websocketfy-Server setup, expecting incoming connections in a `while(true)` loop.

![Running Websocketfy-Server.](https://github.com/marcosivni/websocketfy/blob/main/imgs/WS1.png)
Figure 2. Running  Websocketfy-Server.


### Connecting to the Server

Connecting with the WebSocket Server requires a WebSocket client request (`ws` protocol prefix and a [proper header](https://datatracker.ietf.org/doc/html/rfc6455#section-1.3)).  It can be accomplished with a QWebSocket object connected on a valid hostname and port. This step is performed before the login procedure by [WebHigiia](https://github.com/marcosivni/Webhigiia).


### Example - Messages

Two types of messages are considered by the Websocketfy-Server processing, namely:

- `REQUEST path/filename` - Retrieves `filename` from `path` directory and sends it to the client as a base-64 byte array, and
- `ALTER TABLE`, `CREATE INDEX`, `CREATE METRIC`, `CREATE TABLE`, `DROP INDEX`, `DROP METRIC`, `DROP TABLE`, `DELETE`, `INSERT INTO`, `SELECT`, and `UPDATE` - Valid [extended SQL commands](https://github.com/marcosivni/siren/wiki/Extended-SQL-Grammar) delivered to the SIREN Server.


## Notes 

- Websocketfy-Server is NOT commercial software. **It is built for education and demonstration purposes ONLY!**
- _(C) THIS SOFTWARE IS PROVIDED ``AS IS'' AND ANY EXPRESSED OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHORS OF THIS SOFTWARE OR ITS CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE._



