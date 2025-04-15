#include "test_storage.h"
#include "handlers/OAIVeinApiHandler.h"
#include <timemachinefortest.h>
#include <QTest>
#include <timerfactoryqtfortest.h>
#include <QSignalSpy>
#include <mocktcpnetworkfactory.h>

QTEST_MAIN(test_storage)

void test_storage::initTestCase()
{
    TimerFactoryQtForTest::enableTest();
}

void test_storage::init()
{
    m_tcpSystem = new VeinNet::TcpSystem(VeinTcp::MockTcpNetworkFactory::create());
    VeinNet::NetworkSystem* netSystem = new VeinNet::NetworkSystem();
    netSystem->setOperationMode(VeinNet::NetworkSystem::VNOM_PASS_THROUGH);

    m_testLogger.setupServer(2, 2);
    m_testLogger.loadDatabase();
    m_testLogger.appendEventSystem(netSystem);
    m_testLogger.appendEventSystem(m_tcpSystem);

    m_tcpSystem->startServer(12000);
}

void test_storage::cleanup()
{
    m_testLogger.cleanup();
    m_tcpSystem->deleteLater();
}

void test_storage::access_storage_of_vein_singleton()
{
    VeinTcp::AbstractTcpNetworkFactoryPtr mockedVeinNetworkFactory = VeinTcp::MockTcpNetworkFactory::create();
    VeinEntryPtr veinEntry = VeinEntry::create(mockedVeinNetworkFactory);
    VeinStorage::AbstractDatabase* veinStorageDb = veinEntry->getStorageDb();

    veinEntry->getSubscriptionManager()->subscribeToEntities(QList<int>() << 2);
    TimeMachineObject::feedEventLoop();

    m_testLogger.setComponent(2, "sessionName", QVariant("foo"));

    QVERIFY(veinStorageDb->hasStoredValue(2, "sessionName") == true);
    QString value = veinStorageDb->getStoredValue(2, "sessionName").toString();
    QCOMPARE(value, "foo");
}

void test_storage::get_multiple_values()
{
    VeinTcp::AbstractTcpNetworkFactoryPtr mockedVeinNetworkFactory = VeinTcp::MockTcpNetworkFactory::create();
    m_testLogger.setComponent(0, "Session", QVariant("test-session1.json"));
    m_testLogger.setComponent(2, "sessionName", QVariant("foo"));

    VeinEntryPtr veinEntry = VeinEntry::create(mockedVeinNetworkFactory);

    veinEntry->getSubscriptionManager()->subscribeToEntities(QList<int>() << 0);
    veinEntry->getSubscriptionManager()->subscribeToEntities(QList<int>() << 2);
    TimeMachineObject::feedEventLoop();

    OpenAPI::OAIVeinApiHandler handler(veinEntry);

    QList<OpenAPI::OAIVeinGetRequest> requests;
    requests.append(OpenAPI::OAIVeinGetRequest("{\"EntityId\": 0, \"ComponentName\": \"Session\"}"));
    requests.append(OpenAPI::OAIVeinGetRequest("{\"EntityId\": 2, \"ComponentName\": \"sessionName\"}"));
    requests.append(OpenAPI::OAIVeinGetRequest("{\"EntityId\": 0, \"ComponentName\": \"INF_ModuleInterface\"}"));

    QList<OpenAPI::OAIVeinGetResponse> response = handler.generateBulkAnswer(requests);

    QVERIFY(response.size() == 3);
    QCOMPARE(response[0].getReturnInformation(), "\"test-session1.json\"");
    QCOMPARE(response[1].getReturnInformation(), "\"foo\"");

    QVERIFY(response[2].getReturnInformation().startsWith("{"));
    QVERIFY(response[2].getReturnInformation().length() > 2);
}
