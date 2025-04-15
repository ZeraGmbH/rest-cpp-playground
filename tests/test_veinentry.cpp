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
    m_tcpSystem = new VeinNet::TcpSystem(VeinTcp::MockTcpNetworkFactory::create());
    VeinNet::NetworkSystem* netSystem = new VeinNet::NetworkSystem();
    netSystem->setOperationMode(VeinNet::NetworkSystem::VNOM_PASS_THROUGH);

    m_testLogger.setupServer(2, 2);
    m_testLogger.loadDatabase();
    m_testLogger.appendEventSystem(netSystem);
    m_testLogger.appendEventSystem(m_tcpSystem);

    m_tcpSystem->startServer(12000);
}

void test_veinentry::cleanup()
{
    m_testLogger.cleanup();
    m_tcpSystem->deleteLater();
}

void test_veinentry::setToVeinTwice()
{
    VeinTcp::AbstractTcpNetworkFactoryPtr mockedVeinNetworkFactory = VeinTcp::MockTcpNetworkFactory::create();
    VeinEntryPtr veinEntry = VeinEntry::create(mockedVeinNetworkFactory);

    TaskTemplatePtr task = veinEntry->setToVein(2, "sessionName", "foo");

    QSignalSpy spy(task.get(), &TaskSimpleVeinSetter::sigFinish);

    task->start();

    TimeMachineObject::feedEventLoop();

    QVERIFY(spy.length() == 1);
    QCOMPARE(spy[0][0], true);

    TaskTemplatePtr secondTask = veinEntry->setToVein(2, "sessionName", "bar");

    QSignalSpy secondSpy(secondTask.get(), &TaskSimpleVeinSetter::sigFinish);

    secondTask->start();

    TimeMachineObject::feedEventLoop();

    QVERIFY(secondSpy.length() == 1);
    QCOMPARE(secondSpy[0][0], true);

    QVERIFY(veinEntry->getStorageDb()->getStoredValue(2, "sessionName") == "bar");
}
