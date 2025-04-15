#ifndef TEST_SUBSCRIPTION_MANAGER_H
#define TEST_SUBSCRIPTION_MANAGER_H

#include "vn_tcpsystem.h"
#include <testloggersystem.h>
#include <QObject>

class test_subscription_manager : public QObject
{
    Q_OBJECT
private slots:
    void initTestCase();
    void init();
    void cleanup();
    void subscribeValidEntityAndFetchValidComponent();
    void subscribeInvalidEntity();
    void subscribeValidEntityContainingInvalidComponents();
    void subscribeTwice();

private:
    TestLoggerSystem m_testLogger;
    VeinNet::TcpSystem* m_tcpSystem;
};

#endif // TEST_SUBSCRIPTION_MANAGER_H
