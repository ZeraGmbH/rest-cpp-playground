#ifndef TEST_VEINENTRY_H
#define TEST_VEINENTRY_H

#include <testloggersystem.h>
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
    VeinNet::TcpSystem* m_tcpSystem;
    TestLoggerSystem m_testLogger;
};

#endif // TEST_VEINENTRY_H
