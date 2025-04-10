#ifndef HTTPCURLCLIENT_H
#define HTTPCURLCLIENT_H

#include <QProcess>
#include <QJsonObject>
#include <QObject>

class HttpCurlClient : public QObject
{
    Q_OBJECT
public:
    HttpCurlClient();
    void startCurlProcess(QString requestType, QString URL, QStringList headers, const QJsonObject &paramsJson = QJsonObject());
signals:
    void processFinished(QByteArray responseData);
private:
    QProcess m_curlProcess;
};

#endif // HTTPCURLCLIENT_H
