#ifndef TEST_RESTSERVER_H
#define TEST_RESTSERVER_H

#include "httpserversetup.h"
#include "vn_tcpsystem.h"
#include <testloggersystem.h>
#include <QMap>
#include <QObject>

class test_restserver : public QObject
{
    Q_OBJECT
private slots:
    void initTestCase();
    void init();
    void cleanup();

    void getVeinComponent();
    void getVeinComponentInvalidEntityId();
    void getVeinComponentInvalidComponentName();
    void setVeinComponent();
    void setVeinComponentInvalidEntityId();
    void setVeinComponentInvalidComponentName();
    void getBulkVeinComponent();

private:
    QJsonObject createCurlRpcParamJson(int entityId, QString rpcName, QMap<QString, QString> rpcParams);
    QJsonObject convertResponseToJson(QVariant response);
    QJsonObject invokeCurlClient(QString requestType, QStringList headers, int entityId = -1, QString componentName = QString(), const QJsonObject &paramsJsonObj = QJsonObject());
    QJsonArray invokeCurlClientForPost(QString requestType, QStringList headers, QJsonArray paramsJsonArray);
    TestLoggerSystem m_testLogger;
    std::unique_ptr<HttpServerSetup> m_httpServer;
    VeinNet::TcpSystem* m_tcpSystem;
};

#endif // TEST_RESTSERVER
