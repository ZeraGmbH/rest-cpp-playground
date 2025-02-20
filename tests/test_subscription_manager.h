#ifndef TEST_SUBSCRIPTION_MANAGER_H
#define TEST_SUBSCRIPTION_MANAGER_H

#include <QObject>
#include "modulemanagertestrunner.h"

class test_subscription_manager : public QObject
{
    Q_OBJECT
private slots:
    void initTestCase();
    void subscribeValidEntityAndFetchValidComponent();
    void subscribeInvalidEntity();
    void subscribeValidEntityContainingInvalidComponents();
    void subscribeTwice();

private:
    std::unique_ptr<ModuleManagerTestRunner> setupModuleManager(QString config);

    std::unique_ptr<ModuleManagerTestRunner> m_testRunner;
};

#endif // TEST_SUBSCRIPTION_MANAGER_H
