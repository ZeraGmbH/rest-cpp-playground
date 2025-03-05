#include "test_veinentry.h"
#include "qtest.h"
#include "veinentry.h"

#include <mocktcpnetworkfactory.h>
#include <qsignalspy.h>
#include <timemachinefortest.h>
#include <timerfactoryqtfortest.h>
#include <vn_networksystem.h>

QTEST_MAIN(test_veinentry)

void test_veinentry::initTestCase()
{
    TimerFactoryQtForTest::enableTest();
}

void test_veinentry::init()
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

void test_veinentry::cleanup()
{
    m_testRunner.reset();
    m_tcpSystem->deleteLater();
}

void test_veinentry::setToVeinTwice()
{
    VeinTcp::AbstractTcpNetworkFactoryPtr mockedVeinNetworkFactory = VeinTcp::MockTcpNetworkFactory::create();
    VeinEntryPtr veinEntry = VeinEntry::create(mockedVeinNetworkFactory);

    TaskTemplatePtr task = veinEntry->setToVein(1070, "PAR_MeasuringMode", "2LW");

    QSignalSpy spy(task.get(), &TaskSimpleVeinSetter::sigFinish);

    task->start();

    TimeMachineObject::feedEventLoop();

    QVERIFY(spy.length() == 1);
    QCOMPARE(spy[0][0], true);

    TaskTemplatePtr secondTask = veinEntry->setToVein(1070, "PAR_MeasuringMode", "4LW");

    QSignalSpy secondSpy(secondTask.get(), &TaskSimpleVeinSetter::sigFinish);

    secondTask->start();

    TimeMachineObject::feedEventLoop();

    QVERIFY(secondSpy.length() == 1);
    QCOMPARE(secondSpy[0][0], true);

    QVERIFY(veinEntry->getStorageDb()->getStoredValue(1070, "PAR_MeasuringMode") == "4LW");
}
