#ifndef TEST_JSON_CONVERSIONS_H
#define TEST_JSON_CONVERSIONS_H

#include <QObject>

class test_json_conversions : public QObject
{
    Q_OBJECT

private slots:
    void convertFromNumber();
    void convertFromString();
    void convertFromBoolean();
    void convertFromArray();
    void convertFromObject();
    void convertFromModel();
    void detectBadJson();

signals:
};

#endif // TEST_JSON_CONVERSIONS_H
