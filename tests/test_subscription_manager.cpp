#include "test_subscription_manager.h"
#include "handlers/OAIVeinApiHandler.h"
#include "qsignalspy.h"
#include "qtest.h"
#include "qtestcase.h"
#include "veinentrysingleton.h"
#include <mocktcpnetworkfactory.h>
#include <timemachinefortest.h>
#include <timerfactoryqtfortest.h>

QTEST_MAIN(test_subscription_manager)

void test_subscription_manager::initTestCase()
{
    TimerFactoryQtForTest::enableTest();
    const QString sessionConfig = ZeraModules::ModuleManager::getInstalledSessionPath() + "/mt310s2-emob-session-ac.json";
    m_testRunner = setupModuleManager(sessionConfig);
}

void test_subscription_manager::subscribeValidEntityAndFetchValidComponent()
{
    VeinEntrySingleton& veinSingleton = VeinEntrySingleton::getInstance();
    std::shared_ptr<SubscriptionManager> subscriptionManager = veinSingleton.getSubscriptionManager();

    QList<int> subscriptions = QList<int>() << 0;

    subscriptionManager->subscribeToEntities(subscriptions);
    TimeMachineObject::feedEventLoop();

    OpenAPI::OAIVeinApiHandler handler;
    OpenAPI::OAIVeinGetRequest request;
    request.setEntityId(0);
    request.setComponentName("SessionsAvailable");
    QList<OpenAPI::OAIVeinGetResponse> response = handler.generateBulkAnswer(QList<OpenAPI::OAIVeinGetRequest>() << request);

    QCOMPARE(response[0].getReturnInformation(), "[\"mt310s2-meas-session.json\",\"mt310s2-emob-session-ac.json\",\"mt310s2-emob-session-dc.json\",\"mt310s2-dc-session.json\"]");
}

void test_subscription_manager::subscribeInvalidEntity()
{
    VeinEntrySingleton& veinSingleton = VeinEntrySingleton::getInstance();
    std::shared_ptr<SubscriptionManager> subscriptionManager = veinSingleton.getSubscriptionManager();

    QList<int> subscriptions = QList<int>() << 4711;

    subscriptionManager->subscribeToEntities(subscriptions);
    QSignalSpy spy(subscriptionManager.get(), &SubscriptionManager::finishedSubscribing);
    TimeMachineObject::feedEventLoop();

    QVERIFY(spy.length() == 1);
    QCOMPARE(spy[0][0], false);
}

void test_subscription_manager::subscribeValidEntityContainingInvalidComponents()
{
    VeinEntrySingleton& veinSingleton = VeinEntrySingleton::getInstance();
    std::shared_ptr<SubscriptionManager> subscriptionManager = veinSingleton.getSubscriptionManager();

    QList<int> subscriptions = QList<int>() << 0;

    QSignalSpy spy(subscriptionManager.get(), &SubscriptionManager::finishedSubscribing);
    subscriptionManager->subscribeToEntities(subscriptions);

    TimeMachineObject::feedEventLoop();

    QVERIFY(spy.length() == 1);
    QCOMPARE(spy[0][0], true);
}

void test_subscription_manager::subscribeTwice()
{
    VeinEntrySingleton& veinSingleton = VeinEntrySingleton::getInstance();
    std::shared_ptr<SubscriptionManager> subscriptionManager = veinSingleton.getSubscriptionManager();

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

std::unique_ptr<ModuleManagerTestRunner> test_subscription_manager::setupModuleManager(QString config)
{
    VeinTcp::AbstractTcpNetworkFactoryPtr mockedVeinNetworkFactory = VeinTcp::MockTcpNetworkFactory::create();

    std::unique_ptr<ModuleManagerTestRunner> testRunner = std::make_unique<ModuleManagerTestRunner>(config);
    VeinNet::NetworkSystem* netSystem = new VeinNet::NetworkSystem();
    netSystem->setOperationMode(VeinNet::NetworkSystem::VNOM_PASS_THROUGH);

    VeinNet::TcpSystem* tcpSystem = new VeinNet::TcpSystem(mockedVeinNetworkFactory);

    // Order is important.
    testRunner->getModManFacade()->addSubsystem(netSystem);
    testRunner->getModManFacade()->addSubsystem(tcpSystem);

    tcpSystem->startServer(12000);

    VeinEntrySingleton::getInstance(mockedVeinNetworkFactory);
    TimeMachineObject::feedEventLoop();

    return testRunner;
}
