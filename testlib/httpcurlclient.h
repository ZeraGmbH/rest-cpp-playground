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
        const QJsonArray &paramsJsonArray = QJsonArray();
        const QJsonObject &paramsJsonObj = QJsonObject();
    };
    HttpCurlClient();
    void startCurlProcess(CurlArguments curlArgs);
signals:
    void processFinished(QByteArray responseData);
private:
    QProcess m_curlProcess;
};

#endif // HTTPCURLCLIENT_H
