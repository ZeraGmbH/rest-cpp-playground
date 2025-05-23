#include "test_restserver.h"
#include <mocktcpnetworkfactory.h>
#include <signalspywaiterwithprocesstimers.h>
#include <qsignalspy.h>
#include <timemachinefortest.h>
#include <timerfactoryqtfortest.h>
#include <vn_networksystem.h>
#include <testloghelpers.h>
#include "qtest.h"

QTEST_MAIN(test_restserver)

static const QString httpServerAddress = "127.0.0.1";
static const int httpServerPort = 8082;
static const QString httpServerUrl = "http://" + httpServerAddress + ":" + QString::number(httpServerPort);
static const QString veinApiUrl = "/api/v1/Vein/";
static const QString httpBaseUrl = httpServerUrl + veinApiUrl;

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
    QString url = httpBaseUrl + getUrlExtension(2, "EntityName");
    HttpCurlClient::CurlArguments curlArgs {"GET", url, headers, QJsonArray(), QJsonObject()};
    QVariant response = invokeCurlClient(curlArgs);
    QJsonObject responseJson = convertResponseToJson(response);
    QCOMPARE(responseJson.value("status"), 200);
    QCOMPARE(responseJson.value("ReturnInformation"), "\"_LoggingSystem\"");
    QCOMPARE(responseJson.value("EntityId"), 2);
    QCOMPARE(responseJson.value("ComponentName"), "EntityName");
}

void test_restserver::getVeinComponentInvalidEntityId()
{
    QStringList headers = QStringList() << "accept: application/json";
    QString url = httpBaseUrl + getUrlExtension(25, "EntityName");
    HttpCurlClient::CurlArguments curlArgs {"GET", url, headers, QJsonArray(), QJsonObject()};
    QVariant response = invokeCurlClient(curlArgs);
    QJsonObject responseJson = convertResponseToJson(response);
    QCOMPARE(responseJson.value("status"), 422);
}

void test_restserver::getVeinComponentInvalidComponentName()
{
    QStringList headers = QStringList() << "accept: application/json";
    QString url = httpBaseUrl + getUrlExtension(25, "foo");
    HttpCurlClient::CurlArguments curlArgs {"GET", url, headers, QJsonArray(), QJsonObject()};
    QVariant response = invokeCurlClient(curlArgs);
    QJsonObject responseJson = convertResponseToJson(response);
    QCOMPARE(responseJson.value("status"), 422);
}

void test_restserver::setVeinComponent()
{
    QStringList headers = QStringList() << "accept: application/json" << "Content-Type: application/json";
    QJsonObject params;
    params.insert("EntityID", 2);
    params.insert("componentName", "sessionName");
    params.insert("newValue", "ses01");
    HttpCurlClient::CurlArguments curlArgsPut {"PUT", httpBaseUrl, headers, QJsonArray(), params};
    QVariant response = invokeCurlClient(curlArgsPut);
    QJsonObject responseJson = convertResponseToJson(response);
    QCOMPARE(responseJson.value("status"), 200);

    headers = QStringList() << "accept: application/json";
    QString url = httpBaseUrl + getUrlExtension(2, "sessionName");
    HttpCurlClient::CurlArguments curlArgsGet {"GET", url, headers, QJsonArray(), QJsonObject()};
    response = invokeCurlClient(curlArgsGet);
    responseJson = convertResponseToJson(response);
    QCOMPARE(responseJson.value("status"), 200);
    QCOMPARE(responseJson.value("ReturnInformation"), "\"ses01\"");
}

void test_restserver::setVeinComponentInvalidEntityId()
{
    QStringList headers = QStringList() << "accept: application/json" << "Content-Type: application/json";
    QJsonObject params;
    params.insert("EntityID", 25);
    params.insert("componentName", "sessionName");
    params.insert("newValue", "ses01");
    HttpCurlClient::CurlArguments curlArgs {"PUT", httpBaseUrl, headers, QJsonArray(), params};
    QVariant response = invokeCurlClient(curlArgs);
    QJsonObject responseJson = convertResponseToJson(response);
    QCOMPARE(responseJson.value("status"), 422);
}

void test_restserver::setVeinComponentInvalidComponentName()
{
    QStringList headers = QStringList() << "accept: application/json" << "Content-Type: application/json";
    QJsonObject params;
    params.insert("EntityID", 2);
    params.insert("componentName", "foo");
    params.insert("newValue", "ses01");
    HttpCurlClient::CurlArguments curlArgs {"PUT", httpBaseUrl, headers, QJsonArray(), params};
    QVariant response = invokeCurlClient(curlArgs);
    QJsonObject responseJson = convertResponseToJson(response);
    QCOMPARE(responseJson.value("status"), 422);
}

void test_restserver::getBulkVeinComponent()
{
    QStringList headers = QStringList() << "accept: application/json" << "Content-Type: application/json";
    QJsonObject obj;
    obj.insert("EntityId", 2);
    obj.insert("ComponentName", "DatabaseReady");
    QJsonArray params;
    params.append(obj);
    HttpCurlClient::CurlArguments curlArgs {"POST", httpBaseUrl, headers, params, QJsonObject()};
    QJsonArray responseArr = convertResponseToJsonArray(invokeCurlClient(curlArgs));
    QCOMPARE(responseArr.size(), 1);
    QJsonObject responseObj = responseArr.at(0).toObject();
    QCOMPARE(responseObj.value("status"), 200);
    QCOMPARE(responseObj.value("ReturnInformation"), "true");
}

void test_restserver::getBulkVeinComponentInvalidEntityId()
{
    QStringList headers = QStringList() << "accept: application/json" << "Content-Type: application/json";
    QJsonObject obj;
    obj.insert("EntityId", 25);
    obj.insert("ComponentName", "DatabaseReady");
    QJsonArray params;
    params.append(obj);
    HttpCurlClient::CurlArguments curlArgs {"POST", httpBaseUrl, headers, params, QJsonObject()};
    QJsonArray responseArr = convertResponseToJsonArray(invokeCurlClient(curlArgs));
    QCOMPARE(responseArr.size(), 1);
    QJsonObject responseObj = responseArr.at(0).toObject();
    QCOMPARE(responseObj.value("status"), 422);
}

void test_restserver::getBulkVeinComponentInvalidComponentName()
{
    QStringList headers = QStringList() << "accept: application/json" << "Content-Type: application/json";
    QJsonObject obj;
    obj.insert("EntityId", 2);
    obj.insert("ComponentName", "foo");
    QJsonArray params;
    params.append(obj);
    HttpCurlClient::CurlArguments curlArgs {"POST", httpBaseUrl, headers, params, QJsonObject()};
    QJsonArray responseArr = convertResponseToJsonArray(invokeCurlClient(curlArgs));
    QCOMPARE(responseArr.size(), 1);
    QJsonObject responseObj = responseArr.at(0).toObject();
    QCOMPARE(responseObj.value("status"), 422);
}

void test_restserver::getBulkTwoVeinComponentsSameEntity()
{
    QStringList headers = QStringList() << "accept: application/json" << "Content-Type: application/json";
    QJsonArray params;
    QJsonObject obj;
    obj.insert("EntityId", 2);
    obj.insert("ComponentName", "DatabaseReady");
    params.append(obj);
    obj.insert("EntityId", 2);
    obj.insert("ComponentName", "LoggingEnabled");
    params.append(obj);
    HttpCurlClient::CurlArguments curlArgs {"POST", httpBaseUrl, headers, params, QJsonObject()};
    QVariant var = invokeCurlClient(curlArgs);
    QJsonArray responseArr = convertResponseToJsonArray(var);
    QCOMPARE(responseArr.size(), 2);
    QJsonObject responseObj = responseArr.at(0).toObject();
    QCOMPARE(responseObj.value("status"), 200);
    QCOMPARE(responseObj.value("ReturnInformation"), "true");

    responseObj = responseArr.at(1).toObject();
    QCOMPARE(responseObj.value("status"), 200);
    QCOMPARE(responseObj.value("ReturnInformation"), "false");
}

void test_restserver::getBulkTwoVeinComponentsDifferentEntities()
{
    QStringList headers = QStringList() << "accept: application/json" << "Content-Type: application/json";
    QJsonArray params;
    QJsonObject obj;
    obj.insert("EntityId", 2);
    obj.insert("ComponentName", "DatabaseReady");
    params.append(obj);
    obj.insert("EntityId", 0);
    obj.insert("ComponentName", "Session");
    params.append(obj);
    HttpCurlClient::CurlArguments curlArgs {"POST", httpBaseUrl, headers, params, QJsonObject()};
    QVariant var = invokeCurlClient(curlArgs);
    QJsonArray responseArr = convertResponseToJsonArray(var);
    QCOMPARE(responseArr.size(), 2);
    QJsonObject responseObj = responseArr.at(0).toObject();
    QCOMPARE(responseObj.value("status"), 200);
    QCOMPARE(responseObj.value("ReturnInformation"), "true");

    responseObj = responseArr.at(1).toObject();
    QCOMPARE(responseObj.value("status"), 200);
    QCOMPARE(responseObj.value("ReturnInformation"), "\"test-session1.json\"");
}

void test_restserver::getBulkTwoVeinComponentsOneInvalidEntity()
{
    QStringList headers = QStringList() << "accept: application/json" << "Content-Type: application/json";
    QJsonArray params;
    QJsonObject obj;
    obj.insert("EntityId", 2);
    obj.insert("ComponentName", "DatabaseReady");
    params.append(obj);
    obj.insert("EntityId", 50);
    obj.insert("ComponentName", "Session");
    params.append(obj);
    HttpCurlClient::CurlArguments curlArgs {"POST", httpBaseUrl, headers, params, QJsonObject()};
    QVariant var = invokeCurlClient(curlArgs);
    QJsonArray responseArr = convertResponseToJsonArray(var);
    QCOMPARE(responseArr.size(), 2);
    QJsonObject responseObj = responseArr.at(0).toObject();
    QCOMPARE(responseObj.value("status"), 200);
    QCOMPARE(responseObj.value("ReturnInformation"), "true");

    responseObj = responseArr.at(1).toObject();
    QCOMPARE(responseObj.value("status"), 422);
    QCOMPARE(responseObj.value("ReturnInformation"), "\"Timeout or not existing entity or component\"");
}

void test_restserver::invokeRPCInvalidEntityId()
{
    m_testLogger.setComponent(dataLoggerEntityId, "sessionName", "DbTestSession1");
    m_testLogger.setComponent(dataLoggerEntityId, "guiContext", "ZeraGuiActualValues");
    m_testLogger.setComponent(dataLoggerEntityId, "currentContentSets", QVariantList() << "ZeraAll");
    m_testLogger.setComponentValues(1);
    m_testLogger.startLogging("DbTestSession1", "foo");
    m_testLogger.setComponentValues(2);
    m_testLogger.stopLogging();

    QStringList headers = QStringList() << "accept: application/json" << "Content-Type: application/json";
    QMap<QString, QString> rpcParams;
    rpcParams.insert("p_session", "\"DbTestSession1\"");
    QJsonObject curlParams = createCurlRpcParamJson(2222, "RPC_displaySessionsInfos", rpcParams);

    HttpCurlClient::CurlArguments curlArgs {"POST", httpBaseUrl + "rpc/", headers, QJsonArray(), curlParams};
    QVariant response = invokeCurlClient(curlArgs);

    QFile file(":/rpc-responses/RPC_displaySessionsInfos_InvalidEntityId.json");
    QVERIFY(file.open(QFile::ReadOnly));
    QByteArray jsonExpected = file.readAll();
    QByteArray jsonDumped = response.toByteArray();
    QVERIFY(TestLogHelpers::compareAndLogOnDiff(jsonExpected, jsonDumped));
}

void test_restserver::invokeRPCInvalidRPCName()
{
    QStringList headers = QStringList() << "accept: application/json" << "Content-Type: application/json";
    QMap<QString, QString> rpcParams;
    rpcParams.insert("p_session", "DbTestSession1");
    QJsonObject curlParams = createCurlRpcParamJson(2, "foo", rpcParams);

    HttpCurlClient::CurlArguments curlArgs {"POST", httpBaseUrl + "rpc/", headers, QJsonArray(), curlParams};
    QVariant response = invokeCurlClient(curlArgs);

    QFile file(":/rpc-responses/RPC_displaySessionsInfos_InvalidRPCName.json");
    QVERIFY(file.open(QFile::ReadOnly));
    QByteArray jsonExpected = file.readAll();
    QByteArray jsonDumped = response.toByteArray();
    QVERIFY(TestLogHelpers::compareAndLogOnDiff(jsonExpected, jsonDumped));
}

void test_restserver::invokeRPCInvalidRPCParams()
{
    QStringList headers = QStringList() << "accept: application/json" << "Content-Type: application/json";
    QMap<QString, QString> rpcParams;
    rpcParams.insert("p_session", "bar");
    QJsonObject curlParams = createCurlRpcParamJson(2, "RPC_displaySessionsInfos", rpcParams);

    HttpCurlClient::CurlArguments curlArgs {"POST", httpBaseUrl + "rpc/", headers, QJsonArray(), curlParams};
    QVariant response = invokeCurlClient(curlArgs);

    QFile file(":/rpc-responses/RPC_displaySessionsInfos_InvalidRPCParams.json");
    QVERIFY(file.open(QFile::ReadOnly));
    QByteArray jsonExpected = file.readAll();
    QByteArray jsonDumped = response.toByteArray();
    QVERIFY(TestLogHelpers::compareAndLogOnDiff(jsonExpected, jsonDumped));
}

void test_restserver::invokeRPCWithJsonResponse()
{
    m_testLogger.setComponent(dataLoggerEntityId, "sessionName", "DbTestSession1");
    m_testLogger.setComponent(dataLoggerEntityId, "guiContext", "ZeraGuiActualValues");
    m_testLogger.setComponent(dataLoggerEntityId, "currentContentSets", QVariantList() << "ZeraAll");
    m_testLogger.setComponentValues(1);
    m_testLogger.startLogging("DbTestSession1", "foo");
    m_testLogger.setComponentValues(2);
    m_testLogger.stopLogging();

    QStringList headers = QStringList() << "accept: application/json" << "Content-Type: application/json";
    QMap<QString, QString> rpcParams;
    rpcParams.insert("p_session", "DbTestSession1");
    QJsonObject curlParams = createCurlRpcParamJson(2, "RPC_displaySessionsInfos", rpcParams);

    HttpCurlClient::CurlArguments curlArgs {"POST", httpBaseUrl + "rpc/", headers, QJsonArray(), curlParams};
    QVariant response = invokeCurlClient(curlArgs);

    QFile file(":/rpc-responses/RPC_displaySessionsInfos.json");
    QVERIFY(file.open(QFile::ReadOnly));
    QByteArray jsonExpected = file.readAll();
    QByteArray jsonDumped = response.toByteArray();
    QVERIFY(TestLogHelpers::compareAndLogOnDiff(jsonExpected, jsonDumped));
}

void test_restserver::invokeRPCWithBoolResponse()
{
    QStringList headers = QStringList() << "accept: application/json" << "Content-Type: application/json";
    QMap<QString, QString> rpcParams;
    rpcParams.insert("p_session", "DbTestSession1");
    QJsonObject curlParams = createCurlRpcParamJson(2, "RPC_deleteSession", rpcParams);

    HttpCurlClient::CurlArguments curlArgs {"POST", httpBaseUrl + "rpc/", headers, QJsonArray(), curlParams};
    QJsonObject responseJson = convertResponseToJson(invokeCurlClient(curlArgs));
    QCOMPARE(responseJson.value("ReturnInformation").toString(), "true");
    QCOMPARE(responseJson.value("status"), 200);

    rpcParams.clear();
    rpcParams.insert("p_session", "foo");
    curlParams = createCurlRpcParamJson(2, "RPC_deleteSession", rpcParams);

    curlArgs.paramsJsonObj = curlParams;
    responseJson = convertResponseToJson(invokeCurlClient(curlArgs));
    QCOMPARE(responseJson.value("ReturnInformation").toString(), "\"Select an existing session\"");
    QCOMPARE(responseJson.value("status"), 422);
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
    curlParam.insert("Parameters", rpcParamsJson);
    return curlParam;
}

QJsonObject test_restserver::convertResponseToJson(QVariant response)
{
    return QJsonDocument::fromJson(response.toByteArray()).object();
}

QJsonArray test_restserver::convertResponseToJsonArray(QVariant response)
{
    return QJsonDocument::fromJson(response.toByteArray()).array();
}

QVariant test_restserver::invokeCurlClient(HttpCurlClient::CurlArguments curlArgs)
{
    HttpCurlClient curlProcess;
    QSignalSpy spy(&curlProcess, &HttpCurlClient::processFinished);
    curlProcess.startCurlProcess(curlArgs);
    SignalSpyWaiterWithProcessTimers::waitForSignals(&spy, 1, 1000);
    return spy[0][0];
}

QString test_restserver::getUrlExtension(int entityId, QString componentName)
{
    return "?entity_id=" + QString::number(entityId) + "&component_name=" + componentName;
}
