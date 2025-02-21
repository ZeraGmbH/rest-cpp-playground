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

void test_subscription_manager::subscribeValidEntityAndFetchValidComponent()
{
    const QString sessionConfig = ZeraModules::ModuleManager::getInstalledSessionPath() + "/mt310s2-emob-session-ac.json";
    ModuleManagerTestRunner testRunner(sessionConfig);
    VeinTcp::AbstractTcpNetworkFactoryPtr mockedVeinNetworkFactory = VeinTcp::MockTcpNetworkFactory::create();
    VeinNet::NetworkSystem* netSystem = new VeinNet::NetworkSystem();
    netSystem->setOperationMode(VeinNet::NetworkSystem::VNOM_PASS_THROUGH);
    VeinNet::TcpSystem* tcpSystem = new VeinNet::TcpSystem(mockedVeinNetworkFactory);
    testRunner.getModManFacade()->addSubsystem(netSystem);
    testRunner.getModManFacade()->addSubsystem(tcpSystem);

    tcpSystem->startServer(12000);

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
    tcpSystem->deleteLater(); // remove tcpsystem as it is used in modulemanager and does not clean up properly
}

void test_subscription_manager::subscribeInvalidEntity()
{
    const QString sessionConfig = ZeraModules::ModuleManager::getInstalledSessionPath() + "/mt310s2-emob-session-ac.json";
    ModuleManagerTestRunner testRunner(sessionConfig);
    VeinTcp::AbstractTcpNetworkFactoryPtr mockedVeinNetworkFactory = VeinTcp::MockTcpNetworkFactory::create();
    VeinNet::NetworkSystem* netSystem = new VeinNet::NetworkSystem();
    netSystem->setOperationMode(VeinNet::NetworkSystem::VNOM_PASS_THROUGH);
    VeinNet::TcpSystem* tcpSystem = new VeinNet::TcpSystem(mockedVeinNetworkFactory);
    testRunner.getModManFacade()->addSubsystem(netSystem);
    testRunner.getModManFacade()->addSubsystem(tcpSystem);

    tcpSystem->startServer(12000);

    VeinEntryPtr veinEntry = VeinEntry::create(mockedVeinNetworkFactory);

    std::shared_ptr<SubscriptionManager> subscriptionManager = veinEntry->getSubscriptionManager();

    QList<int> subscriptions = QList<int>() << 4711;

    subscriptionManager->subscribeToEntities(subscriptions);
    QSignalSpy spy(subscriptionManager.get(), &SubscriptionManager::finishedSubscribing);
    TimeMachineObject::feedEventLoop();

    QVERIFY(spy.length() == 1);
    QCOMPARE(spy[0][0], false);
    tcpSystem->deleteLater(); // remove tcpsystem as it is used in modulemanager and does not clean up properly
}

void test_subscription_manager::subscribeValidEntityContainingInvalidComponents()
{
    const QString sessionConfig = ZeraModules::ModuleManager::getInstalledSessionPath() + "/mt310s2-emob-session-ac.json";
    ModuleManagerTestRunner testRunner(sessionConfig);
    VeinTcp::AbstractTcpNetworkFactoryPtr mockedVeinNetworkFactory = VeinTcp::MockTcpNetworkFactory::create();
    VeinNet::NetworkSystem* netSystem = new VeinNet::NetworkSystem();
    netSystem->setOperationMode(VeinNet::NetworkSystem::VNOM_PASS_THROUGH);
    VeinNet::TcpSystem* tcpSystem = new VeinNet::TcpSystem(mockedVeinNetworkFactory);
    testRunner.getModManFacade()->addSubsystem(netSystem);
    testRunner.getModManFacade()->addSubsystem(tcpSystem);

    tcpSystem->startServer(12000);

    VeinEntryPtr veinEntry = VeinEntry::create(mockedVeinNetworkFactory);

    std::shared_ptr<SubscriptionManager> subscriptionManager = veinEntry->getSubscriptionManager();

    QList<int> subscriptions = QList<int>() << 0;

    QSignalSpy spy(subscriptionManager.get(), &SubscriptionManager::finishedSubscribing);
    subscriptionManager->subscribeToEntities(subscriptions);

    TimeMachineObject::feedEventLoop();

    QVERIFY(spy.length() == 1);
    QCOMPARE(spy[0][0], true);
    tcpSystem->deleteLater(); // remove tcpsystem as it is used in modulemanager and does not clean up properly
}

void test_subscription_manager::subscribeTwice()
{
    const QString sessionConfig = ZeraModules::ModuleManager::getInstalledSessionPath() + "/mt310s2-emob-session-ac.json";
    ModuleManagerTestRunner testRunner(sessionConfig);
    VeinTcp::AbstractTcpNetworkFactoryPtr mockedVeinNetworkFactory = VeinTcp::MockTcpNetworkFactory::create();
    VeinNet::NetworkSystem* netSystem = new VeinNet::NetworkSystem();
    netSystem->setOperationMode(VeinNet::NetworkSystem::VNOM_PASS_THROUGH);
    VeinNet::TcpSystem* tcpSystem = new VeinNet::TcpSystem(mockedVeinNetworkFactory);
    testRunner.getModManFacade()->addSubsystem(netSystem);
    testRunner.getModManFacade()->addSubsystem(tcpSystem);

    tcpSystem->startServer(12000);

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
    tcpSystem->deleteLater(); // remove tcpsystem as it is used in modulemanager and does not clean up properly
}
