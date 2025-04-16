#include "test_restserver.h"
#include "httpcurlclient.h"
#include <mocktcpnetworkfactory.h>
#include <signalspywaiterwithprocesstimers.h>
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
    QStringList headers = QStringList() << "accept: application/json";
    QJsonObject response = invokeCurlClient("GET", headers, 2, "EntityName");
    QCOMPARE(response.value("status"), 200);
    QCOMPARE(response.value("ReturnInformation"), "\"_LoggingSystem\"");
    QCOMPARE(response.value("EntityId"), 2);
    QCOMPARE(response.value("ComponentName"), "EntityName");
}

void test_restserver::getVeinComponentInvalidEntityId()
{
    QStringList headers = QStringList() << "accept: application/json";
    QJsonObject response = invokeCurlClient("GET", headers, 25, "EntityName");
    QCOMPARE(response.value("status"), 422);
}

void test_restserver::getVeinComponentInvalidComponentName()
{
    QStringList headers = QStringList() << "accept: application/json";
    QJsonObject response = invokeCurlClient("GET", headers, 25, "foo");
    QCOMPARE(response.value("status"), 422);
}

void test_restserver::setVeinComponent()
{
    QStringList headers = QStringList() << "accept: application/json" << "Content-Type: application/json";
    QJsonObject params;
    params.insert("EntityID", 2);
    params.insert("componentName", "sessionName");
    params.insert("newValue", "ses01");
    QJsonObject response = invokeCurlClient("PUT", headers, -1, "", params);
    QCOMPARE(response.value("status"), 200);

    headers = QStringList() << "accept: application/json";
    response = invokeCurlClient("GET", headers, 2, "sessionName");
    QCOMPARE(response.value("status"), 200);
    QCOMPARE(response.value("ReturnInformation"), "\"ses01\"");
}

void test_restserver::setVeinComponentInvalidEntityId()
{
    QStringList headers = QStringList() << "accept: application/json" << "Content-Type: application/json";
    QJsonObject params;
    params.insert("EntityID", 25);
    params.insert("componentName", "sessionName");
    params.insert("newValue", "ses01");
    QJsonObject response = invokeCurlClient("PUT", headers, -1, "", params);
    QCOMPARE(response.value("status"), 422);
}

void test_restserver::setVeinComponentInvalidComponentName()
{
    QStringList headers = QStringList() << "accept: application/json" << "Content-Type: application/json";
    QJsonObject params;
    params.insert("EntityID", 2);
    params.insert("componentName", "foo");
    params.insert("newValue", "ses01");
    QJsonObject response = invokeCurlClient("PUT", headers, -1, "", params);
    QCOMPARE(response.value("status"), 422);
}

void test_restserver::getBulkVeinComponent()
{
    QStringList headers = QStringList() << "accept: application/json" << "Content-Type: application/json";
    QJsonObject obj;
    obj.insert("EntityID", 0);
    obj.insert("ComponentName", "Session");
    QJsonArray params;
    params.append(obj);
    QJsonArray response = invokeCurlClientForPost("POST", headers, params);
    //QCOMPARE(response.value("status"), 200);
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

QJsonObject test_restserver::invokeCurlClient(QString requestType, QStringList headers, int entityId, QString componentName, const QJsonObject &paramsJsonObj)
{
    HttpCurlClient curlProcess;
    QSignalSpy spy(&curlProcess, &HttpCurlClient::processFinished);
    QString url = httpServerUrl + veinApiUrl;
    if(entityId != -1)
        url += "?entity_id=" + QString::number(entityId) + "&component_name=" + componentName;

    HttpCurlClient::CurlArguments curlArgs {
        requestType,
        url,
        headers,
        QJsonArray(),
        paramsJsonObj
    };
    curlProcess.startCurlProcess(curlArgs);

    SignalSpyWaiterWithProcessTimers::waitForSignals(&spy, 1, 1000);
    return convertResponseToJson(spy[0][0]);
}

QJsonArray test_restserver::invokeCurlClientForPost(QString requestType, QStringList headers, QJsonArray paramsJsonArray)
{
    HttpCurlClient curlProcess;
    QSignalSpy spy(&curlProcess, &HttpCurlClient::processFinished);
    QString url = httpServerUrl + veinApiUrl;
    HttpCurlClient::CurlArguments curlArgs {
        requestType,
        url,
        headers,
        paramsJsonArray,
        QJsonObject()
    };
    curlProcess.startCurlProcess(curlArgs);
    SignalSpyWaiterWithProcessTimers::waitForSignals(&spy, 1, 1000);
    QVariant response = spy[0][0];
    return response.toJsonArray();
}
