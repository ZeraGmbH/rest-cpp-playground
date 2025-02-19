#ifndef TEST_STORAGE_H
#define TEST_STORAGE_H

#include "modulemanagertestrunner.h"
#include <QObject>

class test_storage : public QObject
{
    Q_OBJECT
private slots:
    void initTestCase();
    void access_storage_of_vein_singleton();

private:
    std::unique_ptr<ModuleManagerTestRunner> setupModuleManager(QString config);

signals:
};

#endif // TEST_STORAGE_H
