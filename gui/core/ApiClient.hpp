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
private:
    QTcpSocket *socket ;
    QByteArray buffer ;

    explicit ApiClient(QObject* parent = nullptr);
    ~ApiClient() = default ;
    
public:
    static ApiClient* instance() ;
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

};

#endif // APICLIENT_HPP
