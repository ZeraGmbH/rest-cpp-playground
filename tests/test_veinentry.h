#ifndef TEST_VEINENTRY_H
#define TEST_VEINENTRY_H

#include "modulemanagertestrunner.h"
#include "vn_tcpsystem.h"
#include <QObject>

class test_veinentry : public QObject
{
    Q_OBJECT
private slots:
    void initTestCase();
    void init();
    void cleanup();
    void setToVeinTwice();

private:
    std::unique_ptr<ModuleManagerTestRunner> m_testRunner;
    VeinNet::TcpSystem* m_tcpSystem;
};

#endif // TEST_VEINENTRY_H
