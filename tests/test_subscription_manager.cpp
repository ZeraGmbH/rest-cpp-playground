#include "test_subscription_manager.h"
#include "handlers/OAIVeinApiHandler.h"
#include "qsignalspy.h"
#include "qtest.h"
#include "qtestcase.h"
#include <mocktcpnetworkfactory.h>
#include <timemachinefortest.h>
#include <timerfactoryqtfortest.h>
#include <modulemanagertestrunner.h>

QTEST_MAIN(test_subscription_manager)

void test_subscription_manager::initTestCase()
{
    TimerFactoryQtForTest::enableTest();
}

void test_subscription_manager::init()
{
    const QString sessionConfig = ZeraModules::ModuleManager::getInstalledSessionPath() + "/mt310s2-emob-session-ac.json";
    m_testRunner = std::make_unique<ModuleManagerTestRunner>(sessionConfig);
    VeinTcp::AbstractTcpNetworkFactoryPtr mockedVeinNetworkFactory = VeinTcp::MockTcpNetworkFactory::create();
    m_tcpSystem = new VeinNet::TcpSystem(mockedVeinNetworkFactory);
    VeinNet::NetworkSystem* netSystem = new VeinNet::NetworkSystem();
    netSystem->setOperationMode(VeinNet::NetworkSystem::VNOM_PASS_THROUGH);

    m_testRunner->getModManFacade()->addSubsystem(netSystem);
    m_testRunner->getModManFacade()->addSubsystem(m_tcpSystem);

    m_tcpSystem->startServer(12000);
}

void test_subscription_manager::cleanup()
{
    m_testRunner.reset();
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

    QCOMPARE(response[0].getReturnInformation(), "[\"mt310s2-meas-session.json\",\"mt310s2-emob-session-ac.json\",\"mt310s2-emob-session-dc.json\",\"mt310s2-dc-session.json\"]");
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
