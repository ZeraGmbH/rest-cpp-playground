#include "httpcurlclient.h"
#include <QJsonDocument>

HttpCurlClient::HttpCurlClient()
{
    QObject::connect(&m_curlProcess, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), [&](){
        emit processFinished(m_curlProcess.readAll());
    });
}

void HttpCurlClient::startCurlProcess(QString requestType, QString URL, QStringList headers, bool postCommand, const QJsonArray &paramsArray, const QJsonObject &paramsJson)
{
    QStringList headersList;
    for(QString header: headers) {
        headersList.append("-H");
        headersList.append(header);
    }
    QStringList arguments = QStringList() << "-X" << requestType << URL << headersList;

    if(postCommand) {
        if(!paramsArray.isEmpty()) {
            arguments.append("-d");
            arguments.append("[");
            for(int i = 0; i < paramsArray.count(); i++) {
                QJsonObject obj = paramsArray.at(i).toObject();
                QJsonDocument tempDoc(obj);
                QString paramsStr = QString(tempDoc.toJson());
                arguments.append(paramsStr);
            }
            arguments.append("]");
        }
    }
    else {
        if(!paramsJson.isEmpty()) {
            QJsonDocument tempDoc(paramsJson);
            QString paramsStr = QString(tempDoc.toJson());
            arguments.append("-d");
            arguments.append(paramsStr);
        }
    }
    m_curlProcess.start("/bin/curl", arguments);
}
