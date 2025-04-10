#ifndef TEST_RESTSERVER_H
#define TEST_RESTSERVER_H

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
private:
    QJsonObject createCurlRpcParamJson(int entityId, QString rpcName, QMap<QString, QString> rpcParams);
    QJsonObject convertResponseToJson(QVariant response);
    TestLoggerSystem m_testLogger;
    VeinNet::TcpSystem* m_tcpSystem;
};

#endif // TEST_RESTSERVER
