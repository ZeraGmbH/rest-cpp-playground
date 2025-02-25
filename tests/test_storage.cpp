#include "test_storage.h"
#include "handlers/OAIVeinApiHandler.h"
#include <modulemanager.h>
#include <modulemanagertestrunner.h>
#include <timemachinefortest.h>
#include <QTest>
#include <demovaluesdspdft.h>
#include <testdspvalues.h>
#include <timerfactoryqtfortest.h>
#include <QSignalSpy>
#include <mocktcpnetworkfactory.h>

QTEST_MAIN(test_storage)

void test_storage::initTestCase()
{
    TimerFactoryQtForTest::enableTest();
}

void test_storage::access_storage_of_vein_singleton()
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
    VeinStorage::AbstractDatabase* veinStorageDb = veinEntry->getStorageDb();

    veinEntry->getSubscriptionManager()->subscribeToEntities(QList<int>() << 1050);
    TimeMachineObject::feedEventLoop();

    constexpr int dftEntityId = 1050;

    TestDspInterfacePtr dspInterface = testRunner.getDspInterface(dftEntityId);
    TestDspValues dspValues(dspInterface->getValueList());
    dspValues.setAllValuesSymmetricAc(230, 5, 0, 50);
    dspValues.fireDftActualValues(dspInterface);
    TimeMachineObject::feedEventLoop();

    QVERIFY(veinStorageDb->hasStoredValue(dftEntityId, "ACT_POL_DFTPN4") == true);
    QList<double> exampleValue = veinStorageDb->getStoredValue(dftEntityId, "ACT_POL_DFTPN4").value<QList<double>>();
    QCOMPARE(exampleValue[0], float(5 * M_SQRT2));
    tcpSystem->deleteLater(); // remove tcpsystem as it is used in modulemanager and does not clean up properly
}

void test_storage::get_multiple_values()
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

    testRunner.setVfComponent(1050, "PAR_Interval", QVariant(2));
    testRunner.setVfComponent(1060, "PAR_Interval", QVariant(3));
    VeinEntryPtr veinEntry = VeinEntry::create(mockedVeinNetworkFactory);

    veinEntry->getSubscriptionManager()->subscribeToEntities(QList<int>() << 1050);
    veinEntry->getSubscriptionManager()->subscribeToEntities(QList<int>() << 1060);
    TimeMachineObject::feedEventLoop();

    OpenAPI::OAIVeinApiHandler handler(veinEntry);

    QList<OpenAPI::OAIVeinGetRequest> requests;
    requests.append(OpenAPI::OAIVeinGetRequest("{\"EntityId\": 1050, \"ComponentName\": \"PAR_Interval\"}"));
    requests.append(OpenAPI::OAIVeinGetRequest("{\"EntityId\": 1060, \"ComponentName\": \"PAR_Interval\"}"));
    requests.append(OpenAPI::OAIVeinGetRequest("{\"EntityId\": 1060, \"ComponentName\": \"INF_ModuleInterface\"}"));

    QList<OpenAPI::OAIVeinGetResponse> response = handler.generateBulkAnswer(requests);

    QVERIFY(response.size() == 3);
    QCOMPARE(response[0].getReturnInformation(), "2");
    QCOMPARE(response[1].getReturnInformation(), "3");
    QVERIFY(response[2].getReturnInformation().startsWith("{"));
    QVERIFY(response[2].getReturnInformation().length() > 2);

    tcpSystem->deleteLater(); // remove tcpsystem as it is used in modulemanager and does not clean up properly
}
