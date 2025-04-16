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
    if(!curlArgs.paramsJsonArray.isEmpty()) {
        arguments.append("-d");
        QJsonDocument tempDoc(curlArgs.paramsJsonArray);
        arguments.append(QString(tempDoc.toJson()));
    }
    if(!curlArgs.paramsJsonObj.isEmpty()) {
        arguments.append("-d");
        QJsonDocument tempDoc(curlArgs.paramsJsonObj);
        arguments.append(QString(tempDoc.toJson()));
    }
    m_curlProcess.start("/bin/curl", arguments);
}
