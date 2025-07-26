#include "ApiClient.hpp"
#include "config/Config.hpp"

#include <string>

ApiClient::ApiClient(QObject *parent)
    : QObject{parent}, socket(new QTcpSocket(this)){
    connect(socket, &QTcpSocket::readyRead, this, &ApiClient::onReadyRead);
    connect(socket, &QTcpSocket::connected, this, &ApiClient::connected);
    connect(socket, &QTcpSocket::disconnected, this, &ApiClient::disconnected);
    connect(socket, &QTcpSocket::errorOccurred, this, &ApiClient::onErrorOccurred);
}

void ApiClient::connectToBackend(){
    Config::load();
    QString serverAddress = QString::fromStdString(Config::get<std::string>("server.address").value()) ;
    int serverPort = Config::get<int>("server.port").value() ;
    qDebug() << "connecting to " << serverAddress << "port" << serverPort ;
    socket->connectToHost(serverAddress, serverPort );
}

void ApiClient::sendMessage(const QJsonObject &msg){
    QJsonDocument doc(msg);
    QByteArray data = doc.toJson(QJsonDocument::Compact) + "\n" ;
    qInfo() << "Sending Message:" << data ;
    if ( socket->state() == QAbstractSocket::ConnectedState ){
        socket->write(data);
    }
}

// slot functions

void ApiClient::onReadyRead() {
    buffer.append(socket->readAll());

    while (true){
        int endMessageIndex = buffer.indexOf('\n');
        if ( endMessageIndex == -1 )  break ;

        QByteArray line = buffer.left(endMessageIndex);
        buffer.remove(0, endMessageIndex + 1);

        QJsonParseError err ;
        QJsonDocument doc = QJsonDocument::fromJson(line, &err);
        if ( err.error == QJsonParseError::NoError && doc.isObject() ){
            qDebug() << "Api Response Received" << doc.object() ;
            emit dataReceived(doc.object());
        } else {
            qWarning() << "Invalid JSON received:" << line ;
        }
    }
}

void ApiClient::onConnected() {
    emit connected();
}

void ApiClient::onDisconnected() {
    emit disconnected();
}

void ApiClient::onErrorOccurred(QAbstractSocket::SocketError socketError) {
    Q_UNUSED(socketError);
    emit errorOccurred(socket->errorString());
}
