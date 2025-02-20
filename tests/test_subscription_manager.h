#ifndef TEST_SUBSCRIPTION_MANAGER_H
#define TEST_SUBSCRIPTION_MANAGER_H

#include <QObject>

class test_subscription_manager : public QObject
{
    Q_OBJECT
private slots:
    void initTestCase();
    void subscribeValidEntityAndFetchValidComponent();
    void subscribeInvalidEntity();
    void subscribeValidEntityContainingInvalidComponents();
    void subscribeTwice();

};

#endif // TEST_SUBSCRIPTION_MANAGER_H
