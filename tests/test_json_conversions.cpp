#include "test_json_conversions.h"
#include <OAIAnyType.h>
#include <QJsonArray>
#include <QJsonObject>
#include "qtest.h"

QTEST_MAIN(test_json_conversions)

void test_json_conversions::convertFromNumber()
{
    QVariant cvt = ::OpenAPI::jsonToVariant("-13.20005E+4");

    QVERIFY(cvt.type() != QVariant::Invalid);
    QCOMPARE(cvt.toDouble(), -132000.5);
}

void test_json_conversions::convertFromString()
{
    QVariant cvt = ::OpenAPI::jsonToVariant("\"jochen\"");

    QVERIFY(cvt.type() != QVariant::Invalid);
    QCOMPARE(cvt.toString(), "jochen");
}

void test_json_conversions::convertFromBoolean()
{
    QVariant cvt = ::OpenAPI::jsonToVariant("true");

    QVERIFY(cvt.type() != QVariant::Invalid);
    QCOMPARE(cvt.toBool(), true);
}

void test_json_conversions::convertFromArray()
{
    QVariant cvt = ::OpenAPI::jsonToVariant("[1,true,\"manns\",null]");
    QVERIFY(cvt.type() != QVariant::Invalid);

    QVariantList lst = cvt.toList();

    QCOMPARE(lst.length(), 4);
    QCOMPARE(lst[0], 1);
    QCOMPARE(lst[1], true);
    QCOMPARE(lst[2], "manns");
    QVERIFY(lst[3].isNull());

    QVERIFY(lst[3].isValid());
    QCOMPARE(lst[3].typeName(), "std::nullptr_t");
}

void test_json_conversions::convertFromObject()
{
    QVariant cvt = ::OpenAPI::jsonToVariant("{\"a\":1,\"b\":\"zera\"}");
    QVERIFY(cvt.type() != QVariant::Invalid);

    QVariantMap map = cvt.toMap();

    QCOMPARE(map["a"], 1);
    QCOMPARE(map["b"], "zera");
}

void test_json_conversions::convertFromModel()
{
    QVariant cvt = ::OpenAPI::jsonToVariant("[1,{\"x\":[2,{\"y\":3}]}]");
    QVERIFY(cvt.type() != QVariant::Invalid);

    QVariantList lst = cvt.toList();

    QCOMPARE(lst.length(), 2);
    QCOMPARE(lst[0], 1);

    lst = lst[1].toMap()["x"].toList();

    QCOMPARE(lst.length(), 2);
    QCOMPARE(lst[0], 2);

    QCOMPARE(lst[1].toMap()["y"], 3);
}

void test_json_conversions::detectBadJson()
{
    QVariant cvt = ::OpenAPI::jsonToVariant("[1,{\"x\":[2,{\"y\":NaN}]}]");

    QVERIFY(cvt.type() == QVariant::Invalid);
}

void test_json_conversions::convertFromJson()
{
    QJsonArray outerArray;
    outerArray.append(1);
        QJsonObject middle;
            QJsonArray innerArray;
            innerArray.append(2);
                QJsonObject inner;
                inner["y"] = 3;
            innerArray.append(inner);
        middle["x"] = innerArray;
    outerArray.append(middle);

    QVariant cvt;
    QVERIFY(::OpenAPI::fromJsonValue(cvt, outerArray));

    QVariantList lst = cvt.toList();

    QCOMPARE(lst.length(), 2);
    QCOMPARE(lst[0], 1);

    lst = lst[1].toMap()["x"].toList();

    QCOMPARE(lst.length(), 2);
    QCOMPARE(lst[0], 2);

    QCOMPARE(lst[1].toMap()["y"], 3);
}

