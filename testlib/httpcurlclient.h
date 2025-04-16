#ifndef HTTPCURLCLIENT_H
#define HTTPCURLCLIENT_H

#include <QProcess>
#include <QJsonObject>
#include <QJsonArray>
#include <QObject>

class HttpCurlClient : public QObject
{
    Q_OBJECT
public:
    struct CurlArguments {
        QString requestType;
        QString URL;
        QStringList headers;
        bool postCommand;
        const QJsonArray &paramsArray = QJsonArray();
        const QJsonObject &paramsJson = QJsonObject();
    };
    HttpCurlClient();
    void startCurlProcess(CurlArguments curlArgs);
signals:
    void processFinished(QByteArray responseData);
private:
    QProcess m_curlProcess;
};

#endif // HTTPCURLCLIENT_H
