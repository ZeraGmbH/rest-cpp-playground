#include "httpcurlclient.h"
#include <QJsonDocument>

HttpCurlClient::HttpCurlClient()
{
    QObject::connect(&m_curlProcess, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), [&](){
        emit processFinished(m_curlProcess.readAll());
    });
}

void HttpCurlClient::startCurlProcess(CurlArguments curlArgs)
{
    QStringList headersList;
    for(QString header: curlArgs.headers) {
        headersList.append("-H");
        headersList.append(header);
    }
    QStringList arguments = QStringList() << "-X" << curlArgs.requestType << curlArgs.URL << headersList;

    if(curlArgs.postCommand) {
        if(!curlArgs.paramsArray.isEmpty()) {
            arguments.append("-d");
            arguments.append("[");
            for(int i = 0; i < curlArgs.paramsArray.count(); i++) {
                QJsonObject obj = curlArgs.paramsArray.at(i).toObject();
                QJsonDocument tempDoc(obj);
                QString paramsStr = QString(tempDoc.toJson());
                arguments.append(paramsStr);
            }
            arguments.append("]");
        }
    }
    else {
        if(!curlArgs.paramsJson.isEmpty()) {
            QJsonDocument tempDoc(curlArgs.paramsJson);
            QString paramsStr = QString(tempDoc.toJson());
            arguments.append("-d");
            arguments.append(paramsStr);
        }
    }
    m_curlProcess.start("/bin/curl", arguments);
}
