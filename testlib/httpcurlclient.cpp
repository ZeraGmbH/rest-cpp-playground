#include "httpcurlclient.h"
#include <QJsonDocument>

HttpCurlClient::HttpCurlClient()
{
    QObject::connect(&m_curlProcess, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), [&](){
        emit processFinished(m_curlProcess.readAll());
    });
}

void HttpCurlClient::startCurlProcess(QString requestType, QString URL, QStringList headers, const QJsonObject &paramsJson)
{
    QStringList headersList;
    for(QString header: headers) {
        headersList.append("-H");
        headersList.append(header);
    }
    QStringList arguments = QStringList() << "-X" << requestType << URL << headersList;
    if(!paramsJson.isEmpty()) {
        QJsonDocument tempDoc(paramsJson);
        QString paramsStr = QString(tempDoc.toJson());
        arguments.append("-d");
        arguments.append(paramsStr);
    }
    m_curlProcess.start("/bin/curl", arguments);
}
