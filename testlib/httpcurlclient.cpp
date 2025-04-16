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
        arguments.append("[");
        for(int i = 0; i < curlArgs.paramsJsonArray.count(); i++)
            arguments.append(jsonObjtoString(curlArgs.paramsJsonArray.at(i).toObject()));
        arguments.append("]");
    }
    if(!curlArgs.paramsJsonObj.isEmpty()) {
        arguments.append("-d");
        arguments.append(jsonObjtoString(curlArgs.paramsJsonObj));
    }
    m_curlProcess.start("/bin/curl", arguments);
}

QString HttpCurlClient::jsonObjtoString(QJsonObject jsonObj)
{
    QJsonDocument tempDoc(jsonObj);
    return QString(tempDoc.toJson());
}
