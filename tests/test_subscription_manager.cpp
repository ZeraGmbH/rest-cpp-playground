#include "test_subscription_manager.h"
#include "handlers/OAIVeinApiHandler.h"
#include "qsignalspy.h"
#include "qtest.h"
#include "qtestcase.h"
#include <mocktcpnetworkfactory.h>
#include <timemachinefortest.h>
#include <timerfactoryqtfortest.h>

QTEST_MAIN(test_subscription_manager)

void test_subscription_manager::initTestCase()
{
    TimerFactoryQtForTest::enableTest();
}

void test_subscription_manager::init()
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

void test_subscription_manager::cleanup()
{
    m_testLogger.cleanup();
    m_tcpSystem->deleteLater();
}

void test_subscription_manager::subscribeValidEntityAndFetchValidComponent()
{
    VeinTcp::AbstractTcpNetworkFactoryPtr mockedVeinNetworkFactory = VeinTcp::MockTcpNetworkFactory::create();
    VeinEntryPtr veinEntry = VeinEntry::create(mockedVeinNetworkFactory);

    std::shared_ptr<SubscriptionManager> subscriptionManager = veinEntry->getSubscriptionManager();

    QList<int> subscriptions = QList<int>() << 0;

    subscriptionManager->subscribeToEntities(subscriptions);
    TimeMachineObject::feedEventLoop();

    OpenAPI::OAIVeinApiHandler handler(veinEntry);
    OpenAPI::OAIVeinGetRequest request;
    request.setEntityId(0);
    request.setComponentName("SessionsAvailable");
    QList<OpenAPI::OAIVeinGetResponse> response = handler.generateBulkAnswer(QList<OpenAPI::OAIVeinGetRequest>() << request);

    QCOMPARE(response[0].getReturnInformation(), "[\"test-session1.json\",\"test-session2.json\"]");
}

void test_subscription_manager::subscribeInvalidEntity()
{
    VeinTcp::AbstractTcpNetworkFactoryPtr mockedVeinNetworkFactory = VeinTcp::MockTcpNetworkFactory::create();
    VeinEntryPtr veinEntry = VeinEntry::create(mockedVeinNetworkFactory);

    std::shared_ptr<SubscriptionManager> subscriptionManager = veinEntry->getSubscriptionManager();

    QList<int> subscriptions = QList<int>() << 4711;

    subscriptionManager->subscribeToEntities(subscriptions);
    QSignalSpy spy(subscriptionManager.get(), &SubscriptionManager::finishedSubscribing);
    TimeMachineObject::feedEventLoop();

    QVERIFY(spy.length() == 1);
    QCOMPARE(spy[0][0], false);
}

void test_subscription_manager::subscribeValidEntityContainingInvalidComponents()
{
    VeinTcp::AbstractTcpNetworkFactoryPtr mockedVeinNetworkFactory = VeinTcp::MockTcpNetworkFactory::create();
    VeinEntryPtr veinEntry = VeinEntry::create(mockedVeinNetworkFactory);

    std::shared_ptr<SubscriptionManager> subscriptionManager = veinEntry->getSubscriptionManager();

    QList<int> subscriptions = QList<int>() << 0;

    QSignalSpy spy(subscriptionManager.get(), &SubscriptionManager::finishedSubscribing);
    subscriptionManager->subscribeToEntities(subscriptions);

    TimeMachineObject::feedEventLoop();

    QVERIFY(spy.length() == 1);
    QCOMPARE(spy[0][0], true);

    OpenAPI::OAIVeinApiHandler handler(veinEntry);
    OpenAPI::OAIVeinGetRequest request;
    request.setEntityId(0);
    request.setComponentName("foo");
    QList<OpenAPI::OAIVeinGetResponse> response = handler.generateBulkAnswer(QList<OpenAPI::OAIVeinGetRequest>() << request);

    QCOMPARE(response[0].getStatus(), 422);
}

void test_subscription_manager::subscribeTwice()
{
    VeinTcp::AbstractTcpNetworkFactoryPtr mockedVeinNetworkFactory = VeinTcp::MockTcpNetworkFactory::create();
    VeinEntryPtr veinEntry = VeinEntry::create(mockedVeinNetworkFactory);

    std::shared_ptr<SubscriptionManager> subscriptionManager = veinEntry->getSubscriptionManager();

    QList<int> subscriptions = QList<int>() << 0;

    QSignalSpy spy(subscriptionManager.get(), &SubscriptionManager::finishedSubscribing);
    subscriptionManager->subscribeToEntities(subscriptions);

    TimeMachineObject::feedEventLoop();

    subscriptionManager->subscribeToEntities(subscriptions);

    TimeMachineObject::feedEventLoop();

    QVERIFY(spy.length() == 2);
    QCOMPARE(spy[0][0], true);
    QCOMPARE(spy[1][0], true);
}
