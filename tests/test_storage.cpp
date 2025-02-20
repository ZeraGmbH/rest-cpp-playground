#include "test_storage.h"
#include "veinentrysingleton.h"
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
    const QString sessionConfig = ZeraModules::ModuleManager::getInstalledSessionPath() + "/mt310s2-emob-session-ac.json";
    m_testRunner = setupModuleManager(sessionConfig);

}

void test_storage::access_storage_of_vein_singleton()
{ 
    VeinEntrySingleton& veinSingleton = VeinEntrySingleton::getInstance();
    VeinStorage::AbstractDatabase* veinStorageDb = veinSingleton.getStorageDb();

    veinSingleton.getSubscriptionManager()->subscribeToEntities(QList<int>() << 1050);
    TimeMachineObject::feedEventLoop();

    constexpr int dftEntityId = 1050;

    TestDspInterfacePtr dspInterface = m_testRunner->getDspInterface(dftEntityId);
    TestDspValues dspValues(dspInterface->getValueList());
    dspValues.setAllValuesSymmetricAc(230, 5, 0, 50);
    dspValues.fireDftActualValues(dspInterface);
    TimeMachineObject::feedEventLoop();

    QVERIFY(veinStorageDb->hasStoredValue(dftEntityId, "ACT_POL_DFTPN4") == true);
    QList<double> exampleValue = veinStorageDb->getStoredValue(dftEntityId, "ACT_POL_DFTPN4").value<QList<double>>();
    QCOMPARE(exampleValue[0], float(5 * M_SQRT2));
}

void test_storage::get_multiple_values()
{
    VeinEntrySingleton::getInstance().getSubscriptionManager()->subscribeToEntities(QList<int>() << 1050 << 1060);
    TimeMachineObject::feedEventLoop();

    VeinStorage::AbstractDatabase* veinStorageDb = VeinEntrySingleton::getInstance().getStorageDb();
    m_testRunner->setVfComponent(1050, "PAR_Interval", QVariant(2));
    m_testRunner->setVfComponent(1060, "PAR_Interval", QVariant(3));
    TimeMachineObject::feedEventLoop();

    OpenAPI::OAIVeinApiHandler handler;

    QList<OpenAPI::OAIVeinGetRequest> requests;
    requests.append(OpenAPI::OAIVeinGetRequest("{\"EntityId\": 1050, \"ComponentName\": \"PAR_Interval\"}"));
    requests.append(OpenAPI::OAIVeinGetRequest("{\"EntityId\": 1060, \"ComponentName\": \"PAR_Interval\"}"));

    QList<OpenAPI::OAIVeinGetResponse> response = handler.generateBulkAnswer(requests);

    QVERIFY(response.size() == 2);
    QCOMPARE(response[0].getReturnInformation(), "2");
    QCOMPARE(response[1].getReturnInformation(), "3");
}

std::unique_ptr<ModuleManagerTestRunner> test_storage::setupModuleManager(QString config)
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
