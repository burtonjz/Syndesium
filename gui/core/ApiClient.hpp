#ifndef APICLIENT_HPP
#define APICLIENT_HPP

#include <QObject>
#include <QTcpSocket>
#include <QAbstractSocket>
#include <QJsonDocument>
#include <QJsonObject>

class ApiClient : public QObject
{
    Q_OBJECT
public:
    explicit ApiClient(QObject *parent = nullptr);
    void connectToBackend();
    void sendMessage(const QJsonObject &obj);

signals:
    void connected();
    void disconnected();
    void dataReceived(const QJsonObject &msg);
    void errorOccurred(const QString &error);

private slots:
    void onReadyRead();
    void onConnected();
    void onDisconnected();
    void onErrorOccurred(QAbstractSocket::SocketError socketError);

private:
    QTcpSocket *socket ;
    QByteArray buffer ;
};

#endif // APICLIENT_HPP
