#ifndef TEST_STORAGE_H
#define TEST_STORAGE_H

#include <QObject>

class test_storage : public QObject
{
    Q_OBJECT
private slots:
    void initTestCase();
    void access_storage_of_vein_singleton();
    void get_multiple_values();
};

#endif // TEST_STORAGE_H
