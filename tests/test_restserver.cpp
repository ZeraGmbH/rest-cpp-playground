#include "test_restserver.h"
#include "httpcurlclient.h"
#include <mocktcpnetworkfactory.h>
#include <signalspywaiter.h>
#include <qsignalspy.h>
#include <timemachinefortest.h>
#include <timerfactoryqtfortest.h>
#include <vn_networksystem.h>
#include "qtest.h"

QTEST_MAIN(test_restserver)

static const QString httpServerAddress = "127.0.0.1";
static const int httpServerPort = 8082;
static const QString httpServerUrl = "http://" + httpServerAddress + ":" + QString::number(httpServerPort);
static const QString veinApiUrl = "/api/v1/Vein/";

void test_restserver::initTestCase()
{
    TimerFactoryQtForTest::enableTest();
}

void test_restserver::init()
{
    m_tcpSystem = new VeinNet::TcpSystem(VeinTcp::MockTcpNetworkFactory::create());
    VeinNet::NetworkSystem* netSystem = new VeinNet::NetworkSystem();
    netSystem->setOperationMode(VeinNet::NetworkSystem::VNOM_PASS_THROUGH);

    m_testLogger.setupServer(2, 2);
    m_testLogger.loadDatabase();
    m_testLogger.appendEventSystem(netSystem);
    m_testLogger.appendEventSystem(m_tcpSystem);

    m_tcpSystem->startServer(12000);

    m_httpServer = std::make_unique<HttpServerSetup>(VeinTcp::MockTcpNetworkFactory::create());
    TimeMachineObject::feedEventLoop();
    m_httpServer->startListening(QHostAddress(httpServerAddress), httpServerPort);
}

void test_restserver::cleanup()
{
    m_httpServer.reset();
    m_testLogger.cleanup();
    m_tcpSystem->deleteLater();
}

void test_restserver::getVeinComponent()
{
    HttpCurlClient curlProcess;
    QSignalSpy spy(&curlProcess, &HttpCurlClient::processFinished);

    QStringList headers = QStringList() << "accept: application/json";
    curlProcess.startCurlProcess("GET", httpServerUrl + veinApiUrl + "?entity_id=2&component_name=EntityName", headers);

    SignalSpyWaiter::waitForSignals(&spy, 1, 100);
    QVERIFY(spy.length() == 1);
    QJsonObject response = convertResponseToJson(spy[0][0]);
    QCOMPARE(response.value("status"), 200);
    QCOMPARE(response.value("ReturnInformation"), "\"_LoggingSystem\"");
}

void test_restserver::getVeinComponentInvalidEntityId()
{
    HttpCurlClient curlProcess;
    QSignalSpy spy(&curlProcess, &HttpCurlClient::processFinished);

    QStringList headers = QStringList() << "accept: application/json";
    curlProcess.startCurlProcess("GET", httpServerUrl + veinApiUrl + "?entity_id=25&component_name=EntityName", headers);

    SignalSpyWaiter::waitForSignals(&spy, 1, 100);
    QVERIFY(spy.length() == 1);
    QJsonObject response = convertResponseToJson(spy[0][0]);
    QCOMPARE(response.value("status"), 422);
}

void test_restserver::getVeinComponentInvalidComponentName()
{
    HttpCurlClient curlProcess;
    QSignalSpy spy(&curlProcess, &HttpCurlClient::processFinished);

    QStringList headers = QStringList() << "accept: application/json";
    curlProcess.startCurlProcess("GET", httpServerUrl + veinApiUrl + "?entity_id=2&component_name=EntityNamee", headers);

    SignalSpyWaiter::waitForSignals(&spy, 1, 100);
    QVERIFY(spy.length() == 1);
    QJsonObject response = convertResponseToJson(spy[0][0]);
    QCOMPARE(response.value("status"), 422);
}

QJsonObject test_restserver::createCurlRpcParamJson(int entityId, QString rpcName, QMap<QString, QString> rpcParams)
{
    QJsonArray rpcParamsJson;
    for(QString paramKey: rpcParams.keys()) {
        QJsonObject temp;
        temp.insert("Key", paramKey);
        temp.insert("Value", rpcParams.value(paramKey));
        rpcParamsJson.append(temp);
    }
    QJsonObject curlParam;
    curlParam.insert("EntityId", entityId);
    curlParam.insert("RpcName", rpcName);
    curlParam.insert("parameters", rpcParamsJson);
    return curlParam;
}

QJsonObject test_restserver::convertResponseToJson(QVariant response)
{
    return QJsonDocument::fromJson(response.toByteArray()).object();
}
