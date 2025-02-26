#ifndef TEST_STORAGE_H
#define TEST_STORAGE_H

#include "modulemanagertestrunner.h"
#include "vn_tcpsystem.h"
#include <QObject>

class test_storage : public QObject
{
    Q_OBJECT
private slots:
    void initTestCase();
    void init();
    void cleanup();
    void access_storage_of_vein_singleton();
    void get_multiple_values();

private:
    std::unique_ptr<ModuleManagerTestRunner> m_testRunner;
    VeinNet::TcpSystem* m_tcpSystem;
};

#endif // TEST_STORAGE_H
